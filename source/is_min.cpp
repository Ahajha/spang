#include <spang/dfs.hpp>
#include <spang/is_min.hpp>
#include <spang/projection.hpp>

#include <algorithm>
#include <cassert>
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
Adds the first DFS projection links
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
				continue;

			const dfs_edge_t new_code{
				.from = 0,
				.to = 1,
				.from_label = vertex.label,
				.edge_label = edge.label,
				.to_label = dst_node.label,
			};

			// Lexicographically compare labels
			if (first_less_than(new_code, min_dfs_code))
				return {};

			// TODO: This is likely redoing some work from the previous comparison, plus
			// we don't need to compare the to and from fields. Maybe [[assume]] can help?
			// Or maybe use a three way lexicographical comparison with just the labels?
			if (new_code == min_dfs_code)
				min_instances.emplace_back(edge, -1);
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

TODO: This is simplified enough that it may be significantly faster if it is just put directly
in is_forwards_min.
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
			const bool has_edge = instance_view.has_edge(edge.id);

			// Similar check to is_backwards_min
			const auto rmp_candidate_edges =
				rightmost_path | std::views::reverse | std::views::drop(2);

			const auto rmp_edge_index =
				std::ranges::find_if(rmp_candidate_edges,
			                         [&instance_view, &edge](const auto edge_index)
			                         {
										 const auto& rmp_edge = instance_view.get_edge(edge_index);
										 return edge.to == rmp_edge.from;
									 });

			const bool is_backwards = rmp_edge_index == rmp_candidate_edges.end();

			return !has_edge && is_backwards;
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
Returns true if the specified backwards DFS code is minimal.
*/
bool is_backwards_min(std::vector<min_dfs_projection_link>& min_instances,
                      const std::size_t instance_start_index, const std::size_t instance_end_index,
                      projection_view& instance_view, const graph_t& min_graph,
                      const std::span<const edge_id_t> rightmost_path,
                      const std::span<const dfs_edge_t> dfs_code_list,
                      const dfs_edge_t& dfs_code_to_verify)
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
			// Start from the rightmost vertex, going towards the root. Skip the first two
			// since it isn't possible to go to them.
			const auto rmp_candidate_edges =
				rightmost_path | std::views::reverse | std::views::drop(2);

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

			dfs_edge_t new_code{
				.from = dfs_code_list[rightmost_path[0]].to,
				.to = dfs_code_list[*rmp_edge_index].from,
				.from_label = last_node.label,
				.edge_label = edge_from_last_node.label,
				.to_label = to_node.label,
			};

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
				min_instances.emplace_back(edge_from_last_node, instance_index);
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
Returns true if the specified forwards DFS code is minimal.
*/
bool is_forwards_min(std::vector<min_dfs_projection_link>& min_instances,
                     const std::size_t instance_start_index, const std::size_t instance_end_index,
                     projection_view& instance_view, const graph_t& min_graph,
                     const std::span<const edge_id_t> rightmost_path,
                     const std::span<const dfs_edge_t> dfs_code_list,
                     const dfs_edge_t& dfs_code_to_verify)
{
	for (auto instance_index = instance_start_index; instance_index < instance_end_index;
	     ++instance_index)
	{
		instance_view.build_min_view_no_has_edge_info(min_graph, min_instances, instance_index);

		const auto check_extensions = [&](const vertex_t& rmp_node, const vertex_id_t node_id)
		{
			for (const auto& edge : rmp_node.edges)
			{
				if (instance_view.has_vertex(edge.from))
				{
					// From already exists, skip
					continue;
				}

				const dfs_edge_t new_code{
					.from = node_id,
					.to = static_cast<vertex_id_t>(dfs_code_list[rightmost_path[0]].to + 1),
					.from_label = rmp_node.label,
					.edge_label = edge.label,
					.to_label = min_graph.vertices[edge.to].label,
				};

				if (forwards_less_than(new_code, dfs_code_to_verify))
				{
					return false;
				}

				assert(new_code.to == dfs_code_to_verify.to);
				if (new_code == dfs_code_to_verify)
				{
					min_instances.emplace_back(edge, instance_index);
				}
			}

			return true;
		};

		// Check each possible edge from each node on the RMP, starting from the rightmost vertex.
		for (const auto rmp_edge_index : rightmost_path | std::views::reverse)
		{
			const auto& rmp_edge = instance_view.get_edge(rmp_edge_index);
			const auto& rmp_node = min_graph.vertices[rmp_edge.to];
			const auto from_id = dfs_code_list[rmp_edge_index].from;

			if (!check_extensions(rmp_node, from_id))
			{
				return false;
			}

			// TODO short circuit if possible
		}
		// This section is the last "iteration" of the above loop.
		// TODO: Double check this logic
		const auto& first_edge = instance_view.get_edge(0);
		const auto& root_node = min_graph.vertices[first_edge.from];
		constexpr vertex_id_t from_id = 0;

		if (!check_extensions(root_node, from_id))
		{
			return false;
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

	std::vector<edge_id_t> rightmost_path{0};
	const auto min_graph = build_min_graph(dfs_code_list);

	if (dfs_code_list.size() == 1)
		return true;

	auto min_instances = get_instances_of_first_dfs_code(dfs_code_list[0], min_graph);

	if (!min_instances.has_value())
		return false;

	std::size_t instance_start_index = 0;
	projection_view instance_view(min_graph.n_edges, min_graph.vertices.size());

	// First code has been validated already
	std::size_t n_codes = 2;
	for (const auto& code : dfs_code_list | std::views::drop(1))
	{
		const std::size_t instance_end_index = min_instances->size();
		if (code.is_backwards())
		{
			if (!is_backwards_min(*min_instances, instance_start_index, instance_end_index,
			                      instance_view, min_graph, rightmost_path, dfs_code_list, code))
				return false;
		}
		else
		{
			if (exists_backwards(*min_instances, instance_start_index, instance_end_index,
			                     instance_view, min_graph, rightmost_path) ||
			    !is_forwards_min(*min_instances, instance_start_index, instance_end_index,
			                     instance_view, min_graph, rightmost_path, dfs_code_list, code))
				return false;

			// TODO can just pass this around everywhere instead :)
			const auto sublist = dfs_code_list.first(n_codes);
			update_rightmost_path(rightmost_path, sublist);
		}

		instance_start_index = instance_end_index;
		++n_codes;
	}

	return true;
}

} // namespace spang
