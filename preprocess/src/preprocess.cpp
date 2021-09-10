#include "preprocess.hpp"
#include <set>
#include <algorithm>

namespace spang
{

// functor for determining if an item in a map is infrequent. Used as a convenience
// to avoid code duplication.
struct prune_infrequent
{
	std::size_t min_freq;
	
	template<class label_t>
	bool operator()(const std::pair<const label_t, std::vector<graph_id_t>>& kv_pair) const
	{
		return kv_pair.second.size() < min_freq;
	}
	
	template<class label_t>
	bool operator()(const std::pair<const label_t, occurrence_count>& kv_pair) const
	{
		return kv_pair.second < min_freq;
	}
};

std::pair<std::map<vertex_label_t, std::vector<graph_id_t>>,
          std::map<edge_label_t, std::vector<graph_id_t>>>
  find_frequent_label_occurrences(const std::vector<parsed_input_graph_t>& graphs,
    std::size_t min_freq)
{
	// Start by creating maps containing all labels, then prune infrequent ones later.
	std::map<vertex_label_t, std::vector<graph_id_t>> freq_vertex_labels;
	std::map<edge_label_t, std::vector<graph_id_t>> freq_edge_labels;
	
	for (const auto& graph : graphs)
	{
		// Temporary sets are necessary to ensure each label is counted at most
		// once per graph.
		{
			std::set<vertex_label_t> vertex_labels;
			for (const auto& vertex : graph.vertices)
			{
				if (vertex_labels.insert(vertex.label).second)
				{
					// Only on the first occurence place into freq_vertex_labels.
					freq_vertex_labels[vertex.label].push_back(graph.id);
				}
			}
		}
		
		{
			std::set<edge_label_t> edge_labels;
			for (const auto& edge : graph.edges)
			{
				if (edge_labels.insert(edge.label).second)
				{
					// Only on the first insertion place into freq_edge_labels.
					freq_edge_labels[edge.label].push_back(graph.id);
				}
			}
		}
	}
	
	// Prune infrequent labels.
	std::erase_if(freq_vertex_labels, prune_infrequent{min_freq});
	std::erase_if(freq_edge_labels, prune_infrequent{min_freq});
	
	return { freq_vertex_labels, freq_edge_labels };
}

std::pair<std::map<vertex_label_t, occurrence_count>,
          std::map<edge_label_t, occurrence_count>>
  find_frequent_labels(const std::vector<parsed_input_graph_t>& graphs,
    std::size_t min_freq)
{
	// Start by creating maps containing all labels, then prune infrequent ones later.
	std::map<vertex_label_t, occurrence_count> freq_vertex_labels;
	std::map<edge_label_t, occurrence_count> freq_edge_labels;
	
	for (const auto& graph : graphs)
	{
		// Temporary sets are necessary to ensure each label is counted at most
		// once per graph.
		{
			std::set<vertex_label_t> vertex_labels;
			for (const auto& vertex : graph.vertices)
			{
				if (vertex_labels.insert(vertex.label).second)
				{
					// Only count the first occurence.
					++freq_vertex_labels[vertex.label];
				}
			}
		}
		
		{
			std::set<edge_label_t> edge_labels;
			for (const auto& edge : graph.edges)
			{
				if (edge_labels.insert(edge.label).second)
				{
					// Only count the first occurence.
					++freq_edge_labels[edge.label];
				}
			}
		}
	}
	
	// Prune infrequent labels.
	std::erase_if(freq_vertex_labels, prune_infrequent{min_freq});
	std::erase_if(freq_edge_labels, prune_infrequent{min_freq});
	
	return { freq_vertex_labels, freq_edge_labels };
}

compact_graph_t::compact_graph_t(const graph_t& source) : id(source.id),
	n_edges(source.n_edges), edges(std::make_unique<edge_t[]>(2 * source.n_edges))
{
	for (auto iter = edges.get(); const auto& src_vert : source.vertices)
	{
		// Construct the vertex
		vertices.emplace_back(src_vert.label, src_vert.id,
			std::span{iter, src_vert.edges.size()});
		
		// Copy the edges from the source graph into this one, and have the iterator
		// point to the next edge after to prepare for the next iteration.
		iter = std::ranges::copy(src_vert.edges, iter).out;
	}
}

template<class value_t>
std::vector<compact_graph_t> prune_infrequent_labels(
	const std::vector<parsed_input_graph_t>& graphs,
	std::map<vertex_label_t, value_t> freq_vertex_labels,
	std::map<edge_label_t, value_t> freq_edge_labels)
{
	std::vector<compact_graph_t> result;
	
	for (const auto& input_graph : graphs)
	{
		// Convert from edge-list format to adjacency list format, and prune infrequent
		// labels while doing so.
		graph_t graph;
		
		// Maps old vertex IDs to new IDs, to account for indexes being offset
		std::unordered_map<vertex_id_t, vertex_id_t> label_map;
		
		for (const auto& vert : input_graph.vertices)
		{
			if (freq_vertex_labels.contains(vert.label))
			{
				label_map[vert.id] = static_cast<vertex_id_t>(graph.vertices.size());
				graph.vertices.emplace_back(vert.label, vert.id);
			}
			// Otherwise, vertex is not frequent, ignore whole vertex
		}
		
		for (const auto& edge : input_graph.edges)
		{
			const auto& vert_to   = input_graph.vertices[edge.to];
			const auto& vert_from = input_graph.vertices[edge.from];
			if (freq_edge_labels.contains(edge.label)
			 && freq_vertex_labels.contains(vert_to.label)
			 && freq_vertex_labels.contains(vert_from.label))
			{
				const auto to_id = label_map[vert_to.id];
				const auto from_id = label_map[vert_from.id];
				graph.add_edge(from_id, edge.label, to_id);
			}
		}
		
		// If the graph does not have any edges, then prune the whole thing.
		if (graph.n_edges != 0)
			result.emplace_back(graph);
	}
	
	return result;
}

}
