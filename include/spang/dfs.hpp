#pragma once

#include <spang/graph.hpp>

namespace spang
{

/*!
Edge structure as used in the gSpan paper.
*/
struct dfs_edge_t
{
	vertex_id_t from;
	vertex_id_t to;
	vertex_label_t from_label;
	edge_label_t edge_label;
	vertex_label_t to_label;

	[[nodiscard]] constexpr bool operator==(const dfs_edge_t&) const = default;

	//! Returns true iff this code represents a forwards edge.
	[[nodiscard]] constexpr bool is_forwards() const { return to > from; }

	//! Returns true iff this code represents a backwards edge.
	[[nodiscard]] constexpr bool is_backwards() const { return to < from; }
};

} // namespace spang
