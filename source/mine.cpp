// Contains most of the high-level gSpan logic

#include <spang/extend.hpp>
#include <spang/is_min.hpp>
#include <spang/preprocess.hpp>
#include <spang/projection.hpp>
#include <spang/report.hpp>

#include <cassert>
#include <ranges>
#include <span>

namespace spang
{

namespace
{
auto count_support(const std::span<const dfs_projection_link>& links) -> std::size_t
{
	// Not sure if this is valid, keep for now
	assert(!links.empty());
	graph_id_t prev_id = links.front().graph_id;
	std::size_t support = 0;

	const auto view = links | std::views::drop(1);
	for (const auto& link : view)
	{
		if (prev_id != link.graph_id)
		{
			prev_id = link.graph_id;
			++support;
		}
	}
	return support;
}

// codes is inout so we can add to the end of it. Might be revisited once we parallelize.
void mine_recurse(const std::span<const compact_graph_t> graphs,
                  const std::span<const dfs_projection_link> projections,
                  std::vector<dfs_edge_t>& codes, const std::size_t codes_support,
                  const std::size_t min_freq)
{
	// The 1s are already known to be minimal. The check is pretty cheap though, otherwise we need
	// to check on the looping thread, which could slow things down.
	const auto is_min_result = is_min(codes);
	if (!is_min_result)
	{
		return;
	}
	const auto& [rightmost_path, min_graph] = *is_min_result;

	report(codes, projections, codes_support);

	const auto extended_projections = extend(graphs, codes, projections, rightmost_path);

	for (const auto& [code, code_projections] : extended_projections)
	{
		// Mini todo: Would we get any benefit from freeing the memory of the infrequent codes now?
		// Also to investigate: Should we do this check here, or is it okay to delay until the
		// recursive call? Gut feeling says it's cheaper to check here.
		const auto support = count_support(code_projections);
		if (support >= min_freq)
		{
			codes.push_back(code);
			mine_recurse(graphs, code_projections, codes, support, min_freq);
			codes.pop_back();
		}
	}
}

} // namespace

void mine(const std::span<const compact_graph_t> graphs, const std::size_t min_freq)
{
	// Construct the inital 1-graphs and their instances
	extension_map one_edge_projections;

	for (const auto& graph : graphs)
	{
		for (const auto& vertex : graph.vertices)
		{
			for (const auto& edge : vertex.edges)
			{
				const dfs_edge_t code{
					.from = 0,
					.to = 1,
					.from_label = vertex.label,
					.edge_label = edge.label,
					.to_label = graph.vertices[edge.to].label,
				};
				one_edge_projections[code].push_back(dfs_projection_link{
					.graph_id = graph.id,
					.edge = edge,
					.prev_link = nullptr,
				});
			}
		}
	}

	// Could maybe do 1-spans instead here? Not sure if this is worth it. Will re-evaluate once we
	// get parallelism.
	std::vector<dfs_edge_t> codes;
	for (const auto& [code, projections] : one_edge_projections)
	{
		// No need to check frequency, we already know these 1-edges are frequent due to the
		// preprocessing.
		// Though we might need the support for reporting, later problem.
		codes.push_back(code);
		const auto support = count_support(projections);
		mine_recurse(graphs, projections, codes, support, min_freq);
		codes.pop_back();
	}
}

} // namespace spang
