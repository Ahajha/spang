#pragma once

#include <spang/dfs.hpp>
#include <spang/graph.hpp>

#include <optional>
#include <span>
#include <utility>

namespace spang
{

/*!
Returns the rightmost path and graph of the DFS code list if
the DFS code sequence is minimal, otherwise returns nothing.
*/
auto is_min(const std::span<const dfs_edge_t> dfs_code_list)
	-> std::optional<std::pair<std::vector<edge_id_t>, graph_t>>;

} // namespace spang
