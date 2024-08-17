#include <spang/dfs.hpp>
#include <spang/is_min.hpp>
#include <spang/projection.hpp>
#include <spang/utility.hpp>

#include <algorithm>
#include <cassert>
#include <limits>
#include <optional>
#include <ranges>
#include <span>
#include <vector>

namespace spang
{

namespace
{

/*!
Build a graph representation of a dfs code list.
*/
graph_t build_min_graph(const std::span<const dfs_edge_t> dfs_code_list)
{
	graph_t min_graph;
	min_graph.id = -1;

	// The last edge either goes to or comes from the last vertex.
	const auto last_node_id = std::max(dfs_code_list.back().to, dfs_code_list.back().from);
	min_graph.vertices.resize(last_node_id + 1);
	min_graph.vertices[0].label = dfs_code_list[0].from_label;
	for (const auto& code : dfs_code_list)
	{
		if (code.is_forwards())
		{
			min_graph.vertices[code.to].label = code.to_label;
		}
		min_graph.add_edge(code.from, code.edge_label, code.to);
	}
	return min_graph;
}

/*!
Compares two potential first edges of a DFS code sequence.
Returns true iff the first edge is smaller than the second.
*/
constexpr bool first_less_than(const dfs_edge_t& dfs1, const dfs_edge_t& dfs2)
{
	// The 'from' and 'to' fields can be ignored, as they will always be 0 and 1, respectively.
	// The first edge is essentially just defined as 3 labels.
	return lexicographic_less(dfs1.from_label, dfs2.from_label, dfs1.edge_label, dfs2.edge_label,
	                          dfs1.to_label, dfs2.to_label);
}

/*!
Adds the first DFS projection links and verifies the first DFS code is minimal.

Returns the links if it is minimal, otherwise nothing.
*/
std::optional<std::vector<min_dfs_projection_link>> get_instances_of_first_dfs_code(
	const dfs_edge_t& min_dfs_code, const graph_t& min_graph)
{
	std::vector<min_dfs_projection_link> min_instances;

	for (const auto& vertex : min_graph.vertices)
	{
		for (const auto& edge : vertex.edges)
		{
			// We are comparing labels of the new DFS code that this would produce with the min dfs
			// code. If the opposite edge is lower, then there is no point comparing this one.
			const auto& dst_node = min_graph.vertices[edge.to];
			if (vertex.label > dst_node.label)
			{
				continue;
			}

			const dfs_edge_t new_code{
				.from = 0,
				.to = 1,
				.from_label = vertex.label,
				.edge_label = edge.label,
				.to_label = dst_node.label,
			};

			// Lexicographically compare labels
			if (first_less_than(new_code, min_dfs_code))
			{
				return {};
			}

			// TODO: This is likely redoing some work from the previous comparison, plus
			// we don't need to compare the to and from fields. Maybe [[assume]] can help?
			// Or maybe use a three way lexicographical comparison with just the labels?
			if (new_code == min_dfs_code)
			{
				min_instances.push_back(min_dfs_projection_link{
					.edge = edge, .prev_link_index = std::numeric_limits<std::size_t>::max()});
			}
		}
	}

	return min_instances;
}

/*
For backwards edges, check for connections from the rightmost vertex to any other connections
on the rightmost path. (Note that there will not be any connections to the next-to-rightmost vertex)

Backwards edges going towards vertices further down the RMP are considered small, so those should
come first.
*/

/*!
Returns true if there are any possible backwards expansions from the rightmost vertex.
*/
bool exists_backwards(const std::span<const min_dfs_projection_link> min_instances,
                      const std::size_t instance_start_index, const std::size_t instance_end_index,
                      projection_view& instance_view, const graph_t& min_graph,
                      const std::span<const edge_id_t> rightmost_path)
{
	for (auto instance_index = instance_start_index; instance_index < instance_end_index;
	     ++instance_index)
	{
		instance_view.build_min_view_no_has_vertex_info(min_graph, min_instances, instance_index);

		const auto& last_edge = instance_view.get_edge(rightmost_path[0]);
		const auto& last_node = min_graph.vertices[last_edge.to];

		const auto is_available_backwards_edge = [&](const edge_t& edge)
		{
			if (instance_view.has_edge(edge.id))
			{
				return false;
			}

			// Similar check to is_backwards_min
			const auto rmp_candidate_edges = rightmost_path | std::views::drop(1);

			const auto rmp_edge_index =
				std::ranges::find_if(rmp_candidate_edges,
			                         [&instance_view, &edge](const auto edge_index)
			                         {
										 const auto& rmp_edge = instance_view.get_edge(edge_index);
										 return edge.to == rmp_edge.from;
									 });

			return rmp_edge_index != rmp_candidate_edges.end();
		};

		if (std::ranges::any_of(last_node.edges, is_available_backwards_edge))
		{
			return true;
		}
	}
	return false;
}

/*!
Compares two potential backwards extensions of the same DFS code sequence.
Returns true iff the first edge is smaller than the second.
*/
constexpr bool backwards_less_than(const dfs_edge_t& dfs1, const dfs_edge_t& dfs2)
{
	// The 'from' and 'from_label' fields can be ignored, as all backwards edge extensions
	// must stem from the rightmost vertex. The 'to_label' field can also be ignored, as
	// if they are different, so is 'to', and them being the same is useless for the
	// comparison.
	return lexicographic_less(dfs1.to, dfs2.to, dfs1.edge_label, dfs2.edge_label);
}

/*!
Returns true if the last DFS code provided is minimal.
*/
bool is_backwards_min(std::vector<min_dfs_projection_link>& min_instances,
                      const std::size_t instance_start_index, const std::size_t instance_end_index,
                      projection_view& instance_view, const graph_t& min_graph,
                      const std::span<const edge_id_t> rightmost_path,
                      const std::span<const dfs_edge_t> dfs_code_list)
{
	for (auto instance_index = instance_start_index; instance_index < instance_end_index;
	     ++instance_index)
	{
		instance_view.build_min_view_no_has_vertex_info(min_graph, min_instances, instance_index);

		const auto& last_edge = instance_view.get_edge(rightmost_path[0]);
		const auto& last_node = min_graph.vertices[last_edge.to];

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
				// Doesn't connect to any RMP vertex, skip
				// Minor todo: Maybe if we build vertex info, this can be checked earlier by
				// checking if the edge is forwards. Then can assert this will never happen after
				// the search.
				continue;
			}

			const auto& rmp_edge = instance_view.get_edge(*rmp_edge_index);
			const auto& to_node = min_graph.vertices[rmp_edge.from];

			const dfs_edge_t new_code{
				.from = dfs_code_list[rightmost_path[0]].to,
				.to = dfs_code_list[*rmp_edge_index].from,
				.from_label = last_node.label,
				.edge_label = edge_from_last_node.label,
				.to_label = to_node.label,
			};

			const auto& dfs_code_to_verify = dfs_code_list.back();

			// get_instances_of_first_dfs_code - similar lexicographical compare issue:
			// might be able to optimize away some of the comparisons
			if (backwards_less_than(new_code, dfs_code_to_verify))
			{
				return false;
			}

			assert(new_code.from == dfs_code_to_verify.from);
			assert(new_code.from_label == dfs_code_to_verify.from_label);
			if (new_code == dfs_code_to_verify)
			{
				min_instances.push_back(min_dfs_projection_link{.edge = edge_from_last_node,
				                                                .prev_link_index = instance_index});
			}
		}
	}
	return true;
}

/*!
Compares two potential forwards extensions of the same DFS code sequence.
Returns true iff the first edge is smaller than the second.
*/
constexpr bool forwards_less_than(const dfs_edge_t& dfs1, const dfs_edge_t& dfs2)
{
	// The 'from_label' fields can be ignored, since if they are different, so will the
	// 'from' fields, and if they are the same, this provides no information to the
	// lexicographic comparison. The 'to' fields are also ignored, since both codes are
	// given as extensions of the same existing code, and thus they will be the same.

	// The 'from' field is 'reversed' in this comparison, as forward edges are considered
	// starting from the rightmost vertex, which has the largest 'from' but should be
	// considered the smallest edge.
	return lexicographic_less(dfs2.from, dfs1.from, dfs1.edge_label, dfs2.edge_label, dfs1.to_label,
	                          dfs2.to_label);
}

/*!
Returns true if the last DFS code provided is minimal.
*/
bool is_forwards_min(std::vector<min_dfs_projection_link>& min_instances,
                     const std::size_t instance_start_index, const std::size_t instance_end_index,
                     projection_view& instance_view, const graph_t& min_graph,
                     const std::span<const edge_id_t> rightmost_path,
                     const std::span<const dfs_edge_t> dfs_code_list)
{
	const auto& dfs_code_to_verify = dfs_code_list.back();

	for (auto instance_index = instance_start_index; instance_index < instance_end_index;
	     ++instance_index)
	{
		instance_view.build_min_view_no_has_edge_info(min_graph, min_instances, instance_index);

		const auto check_extensions = [&](const vertex_t& rmp_node, const vertex_id_t node_id)
		{
			for (const auto& edge : rmp_node.edges)
			{
				if (instance_view.has_vertex(edge.to))
				{
					// To already exists, skip
					continue;
				}

				const dfs_edge_t new_code{
					.from = node_id,
					.to = static_cast<vertex_id_t>(dfs_code_list[rightmost_path[0]].to + 1),
					.from_label = rmp_node.label,
					.edge_label = edge.label,
					.to_label = min_graph.vertices[edge.to].label,
				};

				assert(new_code.to == dfs_code_to_verify.to);
				if (forwards_less_than(new_code, dfs_code_to_verify))
				{
					return false;
				}

				if (new_code == dfs_code_to_verify)
				{
					min_instances.push_back(
						min_dfs_projection_link{.edge = edge, .prev_link_index = instance_index});
				}
			}

			return true;
		};

		// Check each possible edge from each node on the RMP, starting from the rightmost
		// vertex.

		// This section is the first "iteration" of the loop below.
		{
			const auto& last_forwards_edge = instance_view.get_edge(rightmost_path[0]);
			const auto& rightmost_node = min_graph.vertices[last_forwards_edge.to];
			const auto node_id = dfs_code_list[rightmost_path[0]].to;

			if (!check_extensions(rightmost_node, node_id))
			{
				return false;
			}

			// Any other nodes along the RMP would produce larger codes, so no need to check.
			if (dfs_code_to_verify.from == node_id)
			{
				continue;
			}
		}
		for (const auto rmp_edge_index : rightmost_path)
		{
			const auto& rmp_edge = instance_view.get_edge(rmp_edge_index);
			const auto& rmp_node = min_graph.vertices[rmp_edge.from];
			const auto node_id = dfs_code_list[rmp_edge_index].from;

			if (!check_extensions(rmp_node, node_id))
			{
				return false;
			}

			// Any other nodes along the RMP would produce larger codes, so no need to check.
			if (dfs_code_to_verify.from == node_id)
			{
				break;
			}
		}
	}

	return true;
}

/*!
Adjusts the rightmost path after a forwards edge is added.
*/
void update_rightmost_path(std::vector<edge_id_t>& rightmost_path,
                           const std::span<const dfs_edge_t> dfs_code_list)
{
	// The last code to be added was forwards
	assert(dfs_code_list.back().is_forwards());
	int prev_id = -1;
	rightmost_path.clear();
	for (auto index_plus_one = dfs_code_list.size(); index_plus_one > 0; --index_plus_one)
	{
		const auto& code = dfs_code_list[index_plus_one - 1];

		if (code.is_forwards() && (rightmost_path.empty() || prev_id == code.to))
		{
			prev_id = code.from;
			rightmost_path.push_back(static_cast<edge_id_t>(index_plus_one - 1));
		}
	}
}

} // namespace

bool is_min(const std::span<const dfs_edge_t> dfs_code_list)
{
	assert(!dfs_code_list.empty());
	assert(dfs_code_list[0].from == 0);
	assert(dfs_code_list[0].to == 1);
	assert(dfs_code_list[0].from_label <= dfs_code_list[0].to_label);

	std::vector<edge_id_t> rightmost_path{0};
	const auto min_graph = build_min_graph(dfs_code_list);

	if (dfs_code_list.size() == 1)
	{
		return true;
	}

	auto min_instances = get_instances_of_first_dfs_code(dfs_code_list[0], min_graph);

	if (!min_instances.has_value())
	{
		return false;
	}

	std::size_t instance_start_index = 0;
	projection_view instance_view(min_graph.n_edges, min_graph.vertices.size());

	// First code has been validated already
	for (std::size_t n_codes = 2; n_codes <= dfs_code_list.size(); ++n_codes)
	{
		const auto sublist = dfs_code_list.first(n_codes);
		const std::size_t instance_end_index = min_instances->size();
		if (sublist.back().is_backwards())
		{
			if (!is_backwards_min(*min_instances, instance_start_index, instance_end_index,
			                      instance_view, min_graph, rightmost_path, sublist))
			{
				return false;
			}
		}
		else
		{
			if (exists_backwards(*min_instances, instance_start_index, instance_end_index,
			                     instance_view, min_graph, rightmost_path) ||
			    !is_forwards_min(*min_instances, instance_start_index, instance_end_index,
			                     instance_view, min_graph, rightmost_path, sublist))
			{
				return false;
			}

			update_rightmost_path(rightmost_path, sublist);
		}

		instance_start_index = instance_end_index;
	}

	return true;
}

} // namespace spang
