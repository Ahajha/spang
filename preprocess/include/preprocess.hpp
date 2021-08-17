#pragma once

#include "graph.hpp"
#include "parser.hpp"
#include <map>
#include <utility>
#include <vector>
#include <memory>
#include <span>

namespace spang
{
	/*!
	Searches graphs for each vertex and edge label that occurs at least min_freq times.
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
	Searches graphs for each vertex and edge label that occurs at least min_freq times.
	For each frequent label, includes the number of graphs the label occurs in.
	*/
	std::pair<std::map<vertex_label_t, occurrence_count>,
	          std::map<edge_label_t, occurrence_count>>
	  find_frequent_labels(const std::vector<parsed_input_graph_t>& graphs,
	    std::size_t min_freq);
	
	/*!
	Prunes infrequent edges and vertices and converts from a edge list
	to an adjacency list format.
	*/
	std::vector<graph_t> prune_infrequent_labels(
	  const std::vector<parsed_input_graph_t>& graphs,
	  std::map<vertex_label_t, std::vector<graph_id_t>> freq_vertex_labels,
	  std::map<edge_label_t, std::vector<graph_id_t>> freq_edge_labels);
	
	std::vector<graph_t> prune_infrequent_labels(
	  const std::vector<parsed_input_graph_t>& graphs,
	  std::map<vertex_label_t, occurrence_count> freq_vertex_labels,
	  std::map<edge_label_t, occurrence_count> freq_edge_labels);
	
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
		
		compact_graph_t(const graph_t& source);
		
		private:
		
		// Contains all edges in a single allocation, vertices reference this via std::spans.
		std::unique_ptr<edge_t[]> edges;
	};
	
	/*!
	Compacts all given graphs into a compact graph format.
	*/
	std::vector<compact_graph_t> compact_graphs(const std::vector<parsed_input_graph_t>& graphs);
	
	/*!
	Convenience function to perform all preprocessing steps. Finds and prunes infrequent edges,
	then converts to a compact graph representation.
	*/
	std::vector<compact_graph_t> preprocess(const std::vector<parsed_input_graph_t>& graphs,
	  std::size_t min_freq);
}
