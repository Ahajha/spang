#include <spang/projection.hpp>

namespace spang
{

projection_view::projection_view(std::size_t max_edges, std::size_t max_vertices)
	: has_edge_{std::make_unique<bool[]>(max_edges)}, has_vertex_{std::make_unique<bool[]>(
														  max_vertices)},
	  contained_edges{std::make_unique<const edge_t*[]>(max_edges)}, n_contained_edges{0}
{
}

void projection_view::build_view()
{
	// TODO
}

/*!
Builds a view of a min_dfs_projection.
*/
template <bool include_edge_info, bool include_vertex_info>
void projection_view::build_min_view(const graph_t& min_graph,
                                     const std::span<const min_dfs_projection_link> projections,
                                     const std::size_t projection_start_index)
{
	if constexpr (include_edge_info)
	{
		std::fill_n(this->has_edge_.get(), min_graph.n_edges, false);
		this->n_contained_edges = 0;
	}
	if constexpr (include_vertex_info)
	{
		std::fill_n(this->has_vertex_.get(), min_graph.vertices.size(), false);
	}

	auto projection_index = projection_start_index;
	while (projection_index != min_dfs_projection_link::no_link)
	{
		const auto& [edge, next_index] = projections[projection_index];

		if constexpr (include_edge_info)
		{
			this->contained_edges[this->n_contained_edges++] = &edge;
			this->has_edge_[edge.id] = true;
		}
		if constexpr (include_vertex_info)
		{
			this->has_vertex_[edge.from] = true;
			this->has_vertex_[edge.to] = true;
		}

		projection_index = next_index;
	}
}

void projection_view::build_min_view_edges_only(
	const graph_t& min_graph, const std::span<const min_dfs_projection_link> projections,
	const std::size_t projection_start_index)
{
	build_min_view<true, false>(min_graph, projections, projection_start_index);
}

void projection_view::build_min_view_vertices_only(
	const graph_t& min_graph, const std::span<const min_dfs_projection_link> projections,
	const std::size_t projection_start_index)
{
	build_min_view<false, true>(min_graph, projections, projection_start_index);
}

} // namespace spang