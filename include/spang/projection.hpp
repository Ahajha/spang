#pragma once

#include <spang/graph.hpp>
#include <spang/preprocess.hpp>

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
	void build_view(const dfs_projection_link& start, const compact_graph_t& graph);

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

	bool has_edge(const edge_id_t id) const { return has_edge_[id]; }
	bool has_vertex(const vertex_id_t id) const { return vertex_refcounts[id] != 0; }

	//! Gets the nth edge added to the graph.
	//! Indexes correspond to the corresponding DFS code list, so get_edge(i) is
	//! the 'actual' edge in the min graph, while dfs_code_list[i] is the DFS edge.
	const edge_t& get_edge(const edge_id_t id) const
	{
		const auto index = n_contained_edges - id - 1;
		return *contained_edges[index];
	}

  private:
	std::unique_ptr<bool[]> has_edge_;
	// For non-min views, we use refcounts for the optimized algorithm. We cannot use booleans with
	// toggles, since any vertex that is referenced twice would count as unreferenced. We could
	// potentially use that approach if we could distinguish forwards edges, but I don't know if
	// that's possible with the information given. This is effectively a boolean for min views,
	// Maybe another reason to split this class in two, though we may also want to try that approach
	// for those.
	std::unique_ptr<vertex_id_t[]> vertex_refcounts;
	std::unique_ptr<const edge_t*[]> contained_edges;
	std::size_t n_contained_edges{0};

	// Only used for non-min views
	// Todo: Should min projection view be a separate class? Current implementation doesn't require
	// vertex refcounts (just bools), but it may be updated to use the more optimal algorithm.
	const compact_graph_t* contained_graph{nullptr};
	const dfs_projection_link* contained_link{nullptr};

	template <bool include_edge_info, bool include_vertex_info>
	void build_min_view(const graph_t& min_graph,
	                    const std::span<const min_dfs_projection_link> projections,
	                    const std::size_t projection_start_index);
};

} // namespace spang
