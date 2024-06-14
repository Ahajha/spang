#pragma once

#include <spang/dfs.hpp>
#include <spang/preprocess.hpp>
#include <spang/projection.hpp>
#include <spang/utility.hpp>

#include <span>
// TODO probably want a faster map, can also try std::map
#include <unordered_map>

namespace spang
{

struct dfs_edge_hash
{
	std::size_t operator()(const dfs_edge_t& dfs_edge) const
	{
		std::size_t seed{0};
		// Ignoring the from field, since within the same map all from fields will be the same.
		hash_combine(seed, dfs_edge.to);
		hash_combine(seed, dfs_edge.from_label);
		hash_combine(seed, dfs_edge.edge_label);
		// TODO: Assuming we're using the relabeled vertices, does the from + edge label
		// uniquely determine the to label? If so, this can be omitted.
		hash_combine(seed, dfs_edge.to_label);
		return seed;
	}
};

using extension_map =
	std::unordered_map<dfs_edge_t, std::vector<dfs_projection_link>, dfs_edge_hash>;

/*
Find extensions of a dfs code sequence within a given database.
*/
extension_map extend(const std::span<const compact_graph_t> graphs,
                     const std::span<const dfs_edge_t> dfs_code_list,
                     const std::span<const dfs_projection_link> subinstances,
                     const std::span<const dfs_edge_t> rightmost_path);

} // namespace spang
