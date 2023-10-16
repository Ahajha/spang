#pragma once

#include "graph.hpp"
#include "parser.hpp"
#include <map>
#include <memory>
#include <span>
#include <utility>
#include <vector>

namespace spang
{

/*!
Searches graphs for each vertex and edge label that occurs in at least min_freq graphs.
For each frequent label, includes a list of graphs that the label occurs in.
*/
std::pair<std::map<vertex_label_t, std::vector<graph_id_t>>,
          std::map<edge_label_t, std::vector<graph_id_t>>>
find_frequent_label_occurrences(const std::vector<parsed_input_graph_t>& graphs,
                                std::size_t min_freq);

/*!
An occurrence count must be large enough to potentially include all graphs. A graph_id_t
would also be sufficient here.
*/
using occurrence_count = std::size_t;

/*!
Searches graphs for each vertex and edge label that occurs in at least min_freq graphs.
For each frequent label, includes the number of graphs the label occurs in.
*/
std::pair<std::map<vertex_label_t, occurrence_count>, std::map<edge_label_t, occurrence_count>>
find_frequent_labels(const std::vector<parsed_input_graph_t>& graphs, std::size_t min_freq);

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
	int n_edges = 0;
	std::vector<compact_vertex_t> vertices;

	//! Compacts a given graph.
	compact_graph_t(const graph_t& source);

  private:
	// Contains all edges in a single allocation, vertices reference this via std::spans.
	std::unique_ptr<edge_t[]> edges;
};

/*!
Prunes infrequent edges and vertices and converts from a edge list
to an adjacency list format.
(Note: the values in frequency are ignored, only the keys are used.
A std::set would be used, but this is intended to be used with the functions above.)
*/
template <class value_t>
std::vector<compact_graph_t> prune_infrequent_labels(
	const std::vector<parsed_input_graph_t>& graphs,
	const std::map<vertex_label_t, value_t> freq_vertex_labels,
	const std::map<edge_label_t, value_t> freq_edge_labels);

} // namespace spang
