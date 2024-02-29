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
bool exists_backwards(const std::span<const min_dfs_projection_link>& min_instances,
                      const std::size_t instance_start_index, const std::size_t instance_end_index,
                      projection_view& instance_view, const graph_t& min_graph,
                      const std::span<const edge_id_t> rightmost_path)
{
	for (auto instance_index = instance_start_index; instance_index < instance_end_index;
	     ++instance_index)
	{
		instance_view.build_min_view_edges_only(min_graph, min_instances, instance_index);

		const auto& last_edge = instance_view.get_edge(rightmost_path[0]);
		const auto& last_node = min_graph.vertices[last_edge.to];

		const auto is_available_backwards_edge = [&instance_view](const edge_t& edge)
		{
			// Check that the edge does not exist, and it would be backwards if added.
			return !instance_view.has_edge(edge.id) && edge.to < edge.from;
		};

		if (std::ranges::any_of(last_node.edges, is_available_backwards_edge))
		{
			return true;
		}
	}
	return false;
}

/*!
Returns true if the specified backwards DFS code is minimal.
*/
bool is_backwards_min()
{
	// TODO
	return true;
}

/*!
Returns true if the specified forwards DFS code is minimal.
*/
bool is_forwards_min()
{
	// TODO
	return true;
}

/*!
Adjusts the rightmost path after a forwards edge is added.
*/
void update_rightmost_path(std::vector<edge_id_t>& rightmost_path)
{
	(void)rightmost_path;
	// TODO
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

	const auto maybe_min_instances = get_instances_of_first_dfs_code(dfs_code_list[0], min_graph);

	if (!maybe_min_instances.has_value())
		return false;

	std::vector<min_dfs_projection_link> min_instances;
	std::size_t instance_start_index = 0;
	projection_view instance_view(min_graph.n_edges, min_graph.vertices.size());

	// First code has been validated already
	for (const auto& code : dfs_code_list | std::views::drop(1))
	{
		const std::size_t instance_end_index = min_instances.size();
		if (code.is_backwards())
		{
			if (!is_backwards_min())
				return false;
		}
		else
		{
			if (exists_backwards(min_instances, instance_start_index, instance_end_index,
			                     instance_view, min_graph, rightmost_path) ||
			    !is_forwards_min())
				return false;
			update_rightmost_path(rightmost_path);
		}

		instance_start_index = instance_end_index;
	}

	return true;
}

} // namespace spang
