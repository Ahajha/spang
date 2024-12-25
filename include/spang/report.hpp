#pragma once

#include <spang/dfs.hpp>
#include <spang/projection.hpp>

#include <span>

namespace spang
{

//! Report the given code sequence as frequent. Projections and support are provided as extra info.
// Todo: Parent graph? Previous thread? (for multithreading)
void report(const std::span<const dfs_edge_t> codes,
            const std::span<const dfs_projection_link> projections,
            const std::size_t codes_support);

} // namespace spang
