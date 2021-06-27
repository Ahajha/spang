#pragma once

#include <graph>

namespace spang
{

/*!
Performs a lexicographic comparison on two tuples, produced by 'unzipping' the parameters.
Returns true iff the first tuple compares less than or equal to the second. For example,
parameters (1,3,5,2,4,6) would produce the comparison (1,5,4) <= (3,2,6).
*/
template<class T, class... Ts>
[[nodiscard]] constexpr bool lexicographic_leq(const T& t1, const T& t2, const T&... rest)
{
	if constexpr (sizeof...(Ts) == 0) return t1 <= t2;
	else return (t1 != t2) ? (t1 < t2) : lexicographic_leq(rest...);
}

/*!
Performs a lexicographic comparison on two tuples, produced by 'unzipping' the parameters.
Returns true iff the first tuple compares less than the second. For example, parameters
(3,3,4,6,7,8) would produce the comparison (3,4,7) < (3,6,8).
*/
template<class T, class... Ts>
[[nodiscard]] constexpr bool lexicographic_less(const T& t1, const T& t2, const T&... rest)
{
	if constexpr (sizeof...(Ts) == 0) return t1 < t2;
	else return (t1 != t2) ? (t1 < t2) : lexicographic_less(rest...);
}

/*!
Edge structure as used in the gSpan paper.
*/
struct dfs_edge_t
{
	vertex_id_t to;
	vertex_id_t from;
	vertex_label_t from_label;
	edge_label_t edge_label;
	vertex_label_t to_label;
	
	[[nodiscard]] constexpr bool operator==(const dfs_edge_t&) const = default;
	
	//! Returns true iff this code represents a forwards edge.
	[[nodiscard]] constexpr bool is_forwards()  const { return to > from; }
	
	//! Returns true iff this code represents a backwards edge.
	[[nodiscard]] constexpr bool is_backwards() const { return to < from; }
};

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
};

/*!
Compares DFS edges in various situations. A single, large comparison could be used, but
these are written for efficiency, as in all of these cases some data can be safely ignored.

Lambdas are used here as these are used as both comparisons in maps as well as directly.
Functors are slightly less convenient to use directly, and functions used in maps
suffer from type erasure, preventing optimizations.
*/

/*!
Compares two potential first edges of a DFS code sequence.
Returns true iff the first edge is smaller than the second.
*/
constexpr auto first_less_than = [](const dfs_edge_t& dfs1, const dfs_edge_t& dfs2)
{
	// The 'from' and 'to' fields can be ignored, as they will always be 0 and 1, respectively.
	// The first edge is essentially just defined as 3 labels.
	return lexicographic_less(dfs1.from_label, dfs2.from_label,
	                          dfs1.edge_label, dfs2.edge_label,
	                          dfs1.to_label,   dfs2.to_label);
};

/*!
Compares two potential forwards extensions of the same DFS code sequence.
Returns true iff the first edge is smaller than the second.
*/
constexpr auto forwards_less_than = [](const dfs_edge_t& dfs1, const dfs_edge_t& dfs2)
{
	// The 'from_label' fields can be ignored, since if they are different, so will the
	// 'from' fields, and if they are the same, this provides no information to the
	// lexicographic comparison. The 'to' fields are also ignored, since both codes are
	// given as extensions of the same existing code, and thus they will be the same.
	
	// The 'from' field is 'reversed' in this comparison, as forward edges are considered
	// starting from the rightmost vertex, which has the largest 'from' but should be
	// considered the smallest edge.
	return lexicographic_less(dfs2.from,       dfs1.from,
	                          dfs1.edge_label, dfs2.edge_label,
	                          dfs1.to_label,   dfs2.to_label);
};

/*!
Compares two potential backwards extensions of the same DFS code sequence.
Returns true iff the first edge is smaller than the second.
*/
constexpr auto backwards_less_than = [](const dfs_edge_t& dfs1, const dfs_edge_t& dfs2)
{
	// The 'from' and 'from_label' fields can be ignored, as all backwards edge extensions
	// must stem from the rightmost vertex. The 'to_label' field can also be ignored, as
	// if they are different, so is 'to', and them being the same is useless for the
	// comparison.
	return lexicographic_less(dfs1.to,         dfs2.to,
	                          dfs1.edge_label, dfs2.edge_label);
};

}
