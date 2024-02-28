#include <spang/dfs.hpp>
#include <spang/is_min.hpp>

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
	for (const auto& code : dfs_code_list)
	{
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
};

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
				.to = 1,
				.from = 0,
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

} // namespace

bool is_min(const std::span<const dfs_edge_t> dfs_code_list)
{
	assert(!dfs_code_list.empty());
	assert(dfs_code_list[0].from == 0);
	assert(dfs_code_list[0].to == 1);

	std::vector<vertex_id_t> rightmost_path{0};
	const auto min_graph = build_min_graph(dfs_code_list);

	if (dfs_code_list.size() == 1)
		return true;

	const auto maybe_min_instances = get_instances_of_first_dfs_code(dfs_code_list[0], min_graph);

	if (!maybe_min_instances.has_value())
		return false;

	for (const auto& code : dfs_code_list | std::views::drop(1))
	{
		(void)code;
		// TODO
	}

	return true;
}

} // namespace spang
