#include <spang/graph.hpp>
#include <spang/preprocess.hpp>
#include <spang/utility.hpp>

#include <algorithm>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace spang
{

namespace
{

/*!
An occurrence count must be large enough to potentially include all graphs. A graph_id_t
would also be sufficient here.
*/
using occurrence_count = std::size_t;

// functor for determining if an item in a map is infrequent. Used as a
// convenience to avoid code duplication.
struct prune_infrequent
{
	std::size_t min_freq;

	template <class label_t>
	[[nodiscard]] bool operator()(const std::pair<const label_t, occurrence_count>& kv_pair) const
	{
		return kv_pair.second < min_freq;
	}
};

/*!
Searches graphs for each vertex label that occurs in at least min_freq graphs.
For each frequent label, returns the number of graphs it occurs in.
*/
[[nodiscard]] auto find_frequent_vertex_labels(const std::span<const parsed_input_graph_t> graphs,
                                               std::size_t min_freq)
	-> std::map<vertex_label_t, occurrence_count>
{
	// Start by creating maps containing all labels, then prune infrequent ones
	// later.
	std::map<vertex_label_t, occurrence_count> freq_vertex_labels;

	for (const auto& graph : graphs)
	{
		// Temporary sets are necessary to ensure each label is counted at most
		// once per graph.
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

	// Prune infrequent labels.
	std::erase_if(freq_vertex_labels, prune_infrequent{min_freq});

	return freq_vertex_labels;
}

struct combined_edge_label
{
	vertex_label_t from_label, to_label;
	edge_label_t edge_label;

	// Normalize all edges to be one 'direction', i.e. 3 --4-- 5 and 5 --4-- 3 should be the same
	// edge.
	combined_edge_label(vertex_label_t vlabel1, edge_label_t elabel, vertex_label_t vlabel2)
		: from_label{std::min(vlabel1, vlabel2)}, to_label{std::max(vlabel1, vlabel2)}, edge_label{
																							elabel}
	{
	}

	[[nodiscard]] constexpr bool operator==(const combined_edge_label&) const = default;
};

struct combined_edge_label_hash
{
	std::size_t operator()(const combined_edge_label& label) const
	{
		std::size_t seed{0};
		hash_combine(seed, label.from_label);
		hash_combine(seed, label.edge_label);
		hash_combine(seed, label.to_label);
		return seed;
	}
};

[[nodiscard]] auto find_frequent_edge_labels(
	const std::span<const parsed_input_graph_t> graphs,
	std::map<vertex_label_t, occurrence_count> freq_vertex_labels, std::size_t min_freq)
	-> std::unordered_map<combined_edge_label, occurrence_count, combined_edge_label_hash>
{
	// Start by creating maps containing all labels, then prune infrequent ones
	// later.
	std::unordered_map<combined_edge_label, occurrence_count, combined_edge_label_hash>
		freq_edge_labels;

	for (const auto& graph : graphs)
	{
		// Temporary sets are necessary to ensure each label is counted at most
		// once per graph.
		std::unordered_set<combined_edge_label, combined_edge_label_hash> edge_labels;
		for (const auto& edge : graph.edges)
		{
			const auto from_label = graph.vertices[edge.from].label;
			const auto to_label = graph.vertices[edge.to].label;
			if (!freq_vertex_labels.contains(from_label) || !freq_vertex_labels.contains(to_label))
			{
				continue;
			}

			const combined_edge_label combo{from_label, edge.label, to_label};

			if (edge_labels.insert(combo).second)
			{
				// Only count the first occurence.
				++freq_edge_labels[combo];
			}
		}
	}

	// Prune infrequent labels.
	std::erase_if(freq_edge_labels, prune_infrequent{min_freq});

	return freq_edge_labels;
}

} // namespace

compact_graph_t::compact_graph_t(const graph_t& source)
	: id(source.id), n_edges(source.n_edges), edges(std::make_unique<edge_t[]>(2 * source.n_edges))
{
	vertices.reserve(source.vertices.size());
	for (auto iter = edges.get(); const auto& src_vert : source.vertices)
	{
		// Construct the vertex
		vertices.emplace_back(src_vert.label, src_vert.id, std::span{iter, src_vert.edges.size()});

		// Copy the edges from the source graph into this one, and have the
		// iterator point to the next edge after to prepare for the next
		// iteration.
		iter = std::ranges::copy(src_vert.edges, iter).out;
	}
}

[[nodiscard]] auto preprocess(std::vector<parsed_input_graph_t>&& graphs, std::size_t min_freq)
	-> std::vector<compact_graph_t>
{
	const auto frequent_vertex_labels = find_frequent_vertex_labels(graphs, min_freq);

	const auto frequent_edge_labels =
		find_frequent_edge_labels(graphs, frequent_vertex_labels, min_freq);

	// Used for the intermediate conversion step (Adjacency list format, but not compact yet)
	graph_t scratch_graph;

	std::vector<compact_graph_t> result;

	for (auto&& input : graphs)
	{
		for (const auto& vertex : input.vertices)
		{
			if (frequent_vertex_labels.contains(vertex.label))
			{
				scratch_graph.vertices.push_back(vertex_t{
					.label = vertex.label,
					.id = vertex.id,
					.edges = {},
				});
			}
		}

		for (const auto& edge : input.edges)
		{
			const auto from_label = input.vertices[edge.from].label;
			const auto to_label = input.vertices[edge.to].label;
			const combined_edge_label combo{from_label, edge.label, to_label};

			if (frequent_edge_labels.contains(combo))
			{
				scratch_graph.add_edge(edge.from, edge.label, edge.to);
			}
		}

		result.emplace_back(scratch_graph);

		// Save memory as we go
		input.vertices.clear();
		input.edges.clear();

		// Reset for the next iteration
		scratch_graph.vertices.clear();
	}

	return result;
}

} // namespace spang
