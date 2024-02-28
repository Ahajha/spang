#pragma once

#include <spang/dfs.hpp>

#include <span>

namespace spang
{

/*!
Returns true if the DFS code sequence is minimal.
TODO: Should return the rightmost path and min_graph if minimal.
*/
bool is_min(const std::span<const dfs_edge_t> dfs_code_list);

} // namespace spang
