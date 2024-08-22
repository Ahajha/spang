#include <spang/extend.hpp>
#include <spang/projection.hpp>

#include <algorithm>
#include <cstdint>
#include <ranges>

namespace spang
{

namespace
{
/*
Adds candidate backwards edges to the extension map.
*/
void extend_backwards(const dfs_projection_link& subinstance, const projection_view& instance_view,
                      const compact_graph_t& graph, const std::span<const dfs_edge_t> dfs_code_list,
                      const std::span<const edge_id_t> rightmost_path, extension_map& map)
{
	const auto& last_edge = instance_view.get_edge(rightmost_path[0]);
	const auto& last_node = graph.vertices[last_edge.to];

	// TODO lots of overlap with is_backwards_min in is_min.cpp, maybe could be extracted
	for (const auto& edge_from_last_node : last_node.edges)
	{
		if (instance_view.has_edge(edge_from_last_node.id))
		{
			// Only looking for edges we could possibly add, skip existing ones
			continue;
		}

		// Check which RMP vertex this connects to.
		// We are checking the from fields of each edge, so skip the first one
		// since that's where the RMV came from.
		const auto rmp_candidate_edges = rightmost_path | std::views::drop(1);

		const auto rmp_edge_index =
			std::ranges::find_if(rmp_candidate_edges,
		                         [&instance_view, &edge_from_last_node](const auto edge_index)
		                         {
									 const auto& rmp_edge = instance_view.get_edge(edge_index);
									 return edge_from_last_node.to == rmp_edge.from;
								 });

		if (rmp_edge_index == rmp_candidate_edges.end())
		{
			// Maybe could check if this vertex has been added before the search
			continue;
		}

		const auto& rmp_edge = instance_view.get_edge(*rmp_edge_index);

		// Overlap ends here

		const auto& rmp_to_node = graph.vertices[rmp_edge.to];

		// Pre-pruning: If the new edge's label is smaller than the existing (RMP) label, then
		// it could have been added previously, so this will not be a minimum DFS code.
		// Failing that, similarly, if the edge labels are the same then check the node labels.
		// If the last node's label is smaller, then it could have been added before, and thus
		// also would produce a smaller DFS code.
		if (lexicographic_leq(rmp_edge.label, edge_from_last_node.label, rmp_to_node.label,
		                      last_node.label))
		{
			const auto& rmp_from_node = graph.vertices[rmp_edge.from];
			const auto from_id = dfs_code_list[rightmost_path[0]].to;
			const auto to_id = dfs_code_list[*rmp_edge_index].from;
			const dfs_edge_t new_code{
				.from = from_id,
				.to = to_id,
				.from_label = last_node.label,
				.edge_label = edge_from_last_node.label,
				.to_label = rmp_from_node.label,
			};
			map[new_code].push_back(dfs_projection_link{
				.graph_id = graph.id, .edge = edge_from_last_node, .prev_link = &subinstance});
		}
	}
}

/*
Adds candidate forwards edges extending from the rightmost vertex.
*/
void extend_forwards_from_rightmost_vertex(const dfs_projection_link& subinstance,
                                           const projection_view& instance_view,
                                           const compact_graph_t& graph,
                                           const std::span<const dfs_edge_t> dfs_code_list,
                                           const std::span<const edge_id_t> rightmost_path,
                                           extension_map& map)
{
	const auto& last_edge = instance_view.get_edge(rightmost_path[0]);
	const auto& last_node = graph.vertices[last_edge.to];
	const auto min_label = dfs_code_list[0].from_label;
	const auto to_id = dfs_code_list[rightmost_path[0]].to;

	for (const auto& candidate_edge : last_node.edges)
	{
		const auto& to_node = graph.vertices[candidate_edge.to];

		// Pre-pruning: Don't consider vertices if the label is smaller than the current known
		// label. Also don't consider vertices that have already been added.
		if (instance_view.has_vertex(candidate_edge.to) || to_node.label < min_label)
		{
			continue;
		}

		const dfs_edge_t new_code{
			.from = to_id,
			.to = static_cast<vertex_id_t>(to_id + 1),
			.from_label = last_node.label,
			.edge_label = candidate_edge.label,
			.to_label = to_node.label,
		};

		map[new_code].push_back(dfs_projection_link{
			.graph_id = graph.id, .edge = candidate_edge, .prev_link = &subinstance});
	}
}

/*
Adds candidate forwards edge extending from the vertices on the rightmost path (other than the
rightmost vertex).
*/
void extend_forwards_from_rightmost_path(const dfs_projection_link& subinstance,
                                         const projection_view& instance_view,
                                         const compact_graph_t& graph,
                                         const std::span<const dfs_edge_t> dfs_code_list,
                                         const std::span<const edge_id_t> rightmost_path,
                                         extension_map& map)
{
	const auto min_label = dfs_code_list[0].from_label;
	const auto to_id = dfs_code_list[rightmost_path[0]].to;

	for (const auto rmp_index : rightmost_path)
	{
		const auto& rmp_edge = instance_view.get_edge(rmp_index);
		const auto& rmp_edge_from = graph.vertices[rmp_edge.from];
		const auto& rmp_edge_to = graph.vertices[rmp_edge.to];

		for (const auto& candidate_edge : rmp_edge_from.edges)
		{
			const auto& to_node = graph.vertices[candidate_edge.to];

			// Pre-pruning: Similar to extensions from RMV
			if (instance_view.has_vertex(candidate_edge.to) || to_node.label < min_label)
			{
				continue;
			}

			// More pre-pruning: If the new edge would have a lower label than the existing edge
			// coming from the same vertex, then it could have been added earlier to make a smaller
			// DFS code.
			// Similarly, if those labels are equal, the same logic can be applied to the labels at
			// the nodes those edges connect to.
			if (lexicographic_leq(rmp_edge.label, candidate_edge.label, rmp_edge_to.label,
			                      to_node.label))
			{
				const auto from_id = dfs_code_list[rmp_index].from;

				const dfs_edge_t new_code{
					.from = from_id,
					.to = static_cast<vertex_id_t>(to_id + 1),
					.from_label = rmp_edge_from.label,
					.edge_label = candidate_edge.label,
					.to_label = to_node.label,
				};

				map[new_code].push_back(dfs_projection_link{
					.graph_id = graph.id, .edge = candidate_edge, .prev_link = &subinstance});
			}
		}
	}
}
} // namespace

extension_map extend(const std::span<const compact_graph_t> graphs,
                     const std::span<const dfs_edge_t> dfs_code_list,
                     const std::span<const dfs_projection_link> subinstances,
                     const std::span<const edge_id_t> rightmost_path)
{
	extension_map map;

	const auto n_vertices = std::ranges::count_if(dfs_code_list, &dfs_edge_t::is_forwards) + 1;

	projection_view instance_view{dfs_code_list.size(), static_cast<std::size_t>(n_vertices)};

	for (const auto& subinstance : subinstances)
	{
		const auto& graph = graphs[static_cast<std::size_t>(subinstance.graph_id)];
		instance_view.build_view(subinstance, graph);

		extend_backwards(subinstance, instance_view, graph, dfs_code_list, rightmost_path, map);

		extend_forwards_from_rightmost_vertex(subinstance, instance_view, graph, dfs_code_list,
		                                      rightmost_path, map);

		extend_forwards_from_rightmost_path(subinstance, instance_view, graph, dfs_code_list,
		                                    rightmost_path, map);
	}

	return map;
}

} // namespace spang
