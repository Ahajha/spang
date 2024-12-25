#include <spang/projection.hpp>

namespace spang
{

static inline void toggle(bool& b) { b = !b; }

projection_view::projection_view(std::size_t max_edges, std::size_t max_vertices)
	: has_edge_{std::make_unique<bool[]>(max_edges)},
	  vertex_refcounts{std::make_unique<vertex_id_t[]>(max_vertices)},
	  contained_edges{std::make_unique<const edge_t*[]>(max_edges)}
{
}

void projection_view::build_view(const dfs_projection_link& start, const compact_graph_t& graph)
{
	if (contained_graph != &graph)
	{
		// New graph, start from scratch
		std::fill_n(has_edge_.get(), graph.n_edges, false);
		std::fill_n(vertex_refcounts.get(), graph.vertices.size(), static_cast<vertex_id_t>(0));
		n_contained_edges = 0;

		auto* current_link = &start;
		do
		{
			contained_edges[n_contained_edges++] = &(current_link->edge);
			has_edge_[current_link->edge.id] = true;
			++vertex_refcounts[current_link->edge.from];
			++vertex_refcounts[current_link->edge.to];
			current_link = current_link->prev_link;
		}
		while (current_link != nullptr);

		contained_graph = &graph;
	}
	else
	{
		// Has an encoded instance from the same graph.
		// Assume it is the same size and distinct.
		// (allows the do-while)
		// Reuse as much of this as possible.

		auto* new_link = &start;
		auto* old_link = contained_link;

		std::size_t modify_index = 0;

		do
		{
			contained_edges[modify_index++] = &(new_link->edge);

			// Remove old edge
			toggle(has_edge_[old_link->edge.id]);
			--vertex_refcounts[old_link->edge.from];
			--vertex_refcounts[old_link->edge.to];

			// Add new edge
			toggle(has_edge_[new_link->edge.id]);
			++vertex_refcounts[new_link->edge.from];
			++vertex_refcounts[new_link->edge.to];

			// As the code lengths are the same, this will also catch the case
			// where new_dfs and old_dfs end up as nullptr at the same time.
			new_link = new_link->prev_link;
			old_link = old_link->prev_link;

			assert((new_link == nullptr) == (old_link == nullptr));
		}
		while (new_link != old_link);
	}
	contained_link = &start;
}

/*!
Builds a view of a min_dfs_projection.
*/
template <bool include_has_edge_info, bool include_has_vertex_info>
void projection_view::build_min_view(const graph_t& min_graph,
                                     const std::span<const min_dfs_projection_link> projections,
                                     const std::size_t projection_start_index)
{
	// Unconditionally include edge references
	this->n_contained_edges = 0;

	if constexpr (include_has_edge_info)
	{
		std::fill_n(this->has_edge_.get(), min_graph.n_edges, false);
	}
	if constexpr (include_has_vertex_info)
	{
		std::fill_n(this->vertex_refcounts.get(), min_graph.vertices.size(),
		            static_cast<vertex_id_t>(0));
	}

	auto projection_index = projection_start_index;
	while (projection_index != min_dfs_projection_link::no_link)
	{
		const auto& [edge, next_index] = projections[projection_index];

		this->contained_edges[this->n_contained_edges++] = &edge;

		if constexpr (include_has_edge_info)
		{
			this->has_edge_[edge.id] = true;
		}
		if constexpr (include_has_vertex_info)
		{
			this->vertex_refcounts[edge.from] = 1;
			this->vertex_refcounts[edge.to] = 1;
		}

		projection_index = next_index;
	}
}

void projection_view::build_min_view_no_has_vertex_info(
	const graph_t& min_graph, const std::span<const min_dfs_projection_link> projections,
	const std::size_t projection_start_index)
{
	build_min_view<true, false>(min_graph, projections, projection_start_index);
}

void projection_view::build_min_view_no_has_edge_info(
	const graph_t& min_graph, const std::span<const min_dfs_projection_link> projections,
	const std::size_t projection_start_index)
{
	build_min_view<false, true>(min_graph, projections, projection_start_index);
}

} // namespace spang
