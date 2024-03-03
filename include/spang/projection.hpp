#pragma once

#include <spang/graph.hpp>

#include <limits>
#include <memory>
#include <span>

namespace spang
{

/*!
A 'projection' is an instance of a DFS code in an input graph. A projection link is effectively
one edge in this instance. This eventually develops into a tree structure, where multiple new
links may extend out of an existing one, hence the lack of a standard container such as
std::list or std::vector.
*/
struct dfs_projection_link
{
	//! ID of the graph this link is in (not particularly memory efficient, as prev_link will
	//! point to a link with the same ID, but leads to a simpler implementation).
	graph_id_t graph_id;

	//! The actual edge in the graph that this link represents.
	const edge_t& edge;

	//! A non-owning pointer to the previous link in the chain, or nullptr if this is
	//! the first link.
	const dfs_projection_link* prev_link;
};

/*!
A 'min projection' is an instance of a DFS code in its own graph
representation. This (also) eventually develops into a tree structure.
*/
struct min_dfs_projection_link
{
	//! The actual edge in the graph that this link represents.
	const edge_t& edge;

	//! The index of the previous link in the chain, or -1 if this is the first link. This is
	//! different than in 'normal' projection links, since these are intended to be stored in
	//! a vector, in a heap-like format, mainly to reduce memory footprint as well as keep the
	//! data more compact. Also, reallocations would invalidate pointers, so indexes must be
	//! used instead of pointers directly to the elements.
	std::size_t prev_link_index;

	constexpr static std::size_t no_link = std::numeric_limits<std::size_t>::max();
};

/*!
A 'projection view' encodes a (min) projection in a way that information about it is
accessible in constant time, rather than having to iterate over the tree structure each time.
*/
class projection_view
{
  public:
	projection_view(std::size_t max_edges, std::size_t max_vertices);

	/*!
	Builds a view of a dfs_projection.
	*/
	void build_view();

	/*!
	Builds a view of a min_dfs_projection. Does not set information on
	whether the graph has a vertex or not.
	*/
	void build_min_view_no_has_vertex_info(
		const graph_t& min_graph, const std::span<const min_dfs_projection_link> projections,
		const std::size_t projection_start_index);

	/*!
	Builds a view of a min_dfs_projection. Does not set information on
	whether the graph has an edge or not.
	*/
	void build_min_view_no_has_edge_info(const graph_t& min_graph,
	                                     const std::span<const min_dfs_projection_link> projections,
	                                     const std::size_t projection_start_index);

	bool has_edge(edge_id_t id) { return has_edge_[id]; }
	bool has_vertex(vertex_id_t id) { return has_vertex_[id]; }

	const edge_t& get_edge(edge_id_t id) { return *contained_edges[id]; }

  private:
	std::unique_ptr<bool[]> has_edge_;
	std::unique_ptr<bool[]> has_vertex_;
	std::unique_ptr<const edge_t*[]> contained_edges;
	std::size_t n_contained_edges;

	template <bool include_edge_info, bool include_vertex_info>
	void build_min_view(const graph_t& min_graph,
	                    const std::span<const min_dfs_projection_link> projections,
	                    const std::size_t projection_start_index);
};

} // namespace spang