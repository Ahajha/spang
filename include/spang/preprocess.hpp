#pragma once

#include <spang/graph.hpp>
#include <spang/parser.hpp>

#include <map>
#include <memory>
#include <span>
#include <utility>
#include <vector>

namespace spang
{

/*!
Compact graph representation. Micro-optimization to
avoid cache misses while observing a single graph.
*/
struct compact_graph_t
{
	struct compact_vertex_t
	{
		vertex_label_t label;
		vertex_id_t id;
		std::span<edge_t> edges;
	};

	graph_id_t id;
	std::uint32_t n_edges = 0;
	std::vector<compact_vertex_t> vertices;

	//! Compacts a given graph into adjacency list format, given a list of edges.
	//! (The edges in the input graph are ignored.)
	//! Prunes infrequent edges, then removes vertices with no edges.
	//! Relabels vertex indexes in edges as needed.
	//! Assumes at least 1 edge.
	//! vertex_id_to_n_edges and vertex_id_map are used as scratch memory.
	compact_graph_t(const parsed_input_graph_t& input, const std::span<const edge_t> edges,
	                std::vector<vertex_id_t>& vertex_id_to_n_edges,
	                std::vector<vertex_id_t>& vertex_id_map);

  private:
	// Contains all edges in a single allocation, vertices reference this via std::spans.
	std::unique_ptr<edge_t[]> edges;
};

/*!
Prunes edges and vertices that would not be in any frequent 1-edge graphs and converts
from a edge list to an adjacency list format.

Will remove data from graphs as it is converted, in order to prevent two full copies
of the data from residing in memory.
*/
[[nodiscard]] auto preprocess(std::vector<parsed_input_graph_t>&& graphs, std::size_t min_freq)
	-> std::vector<compact_graph_t>;

} // namespace spang
