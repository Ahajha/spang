#include <spang/graph.hpp>
#include <spang/preprocess.hpp>
#include <spang/utility.hpp>

#include <algorithm>
#include <limits>
#include <map>
#include <set> // IWYU pragma: keep (std::set, incorrect lint)
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

compact_graph_t::compact_graph_t(const parsed_input_graph_t& input,
                                 const std::span<const edge_t> input_edges,
                                 std::vector<vertex_id_t>& vertex_id_to_n_edges,
                                 std::vector<vertex_id_t>& vertex_id_map)
	: id{input.id}, n_edges{static_cast<std::uint32_t>(input_edges.size())},
	  edges{std::make_unique<edge_t[]>(2 * input_edges.size())}
{
	vertex_id_to_n_edges.resize(input.vertices.size());
	std::ranges::fill(vertex_id_to_n_edges, vertex_id_t(0));

	// 1: Determine # of edges per vertex
	for (const auto& edge : input_edges)
	{
		++vertex_id_to_n_edges[edge.from];
		++vertex_id_to_n_edges[edge.to];
	}

	// 2: Remap vertex indexes
	vertex_id_map.resize(input.vertices.size());
	vertex_id_t n_vertices{0};
	for (vertex_id_t vertex_id{0}; vertex_id < input.vertices.size(); ++vertex_id)
	{
		if (vertex_id_to_n_edges[vertex_id] > 0)
		{
			assert(n_vertices != std::numeric_limits<vertex_id_t>::max());
			vertex_id_map[vertex_id] = n_vertices++;
		}
		else
		{
			vertex_id_map[vertex_id] = std::numeric_limits<vertex_id_t>::max();
		}
	}

	// 3: Prep vertices
	vertices.reserve(n_vertices);
	auto iter = edges.get();
	for (vertex_id_t vertex_id{0}; vertex_id < input.vertices.size(); ++vertex_id)
	{
		if (vertex_id_to_n_edges[vertex_id] == 0)
		{
			continue;
		}
		const auto& src_vert = input.vertices[vertex_id];
		assert(src_vert.id == vertex_id);
		const std::span edge_list{iter, vertex_id_to_n_edges[vertex_id]};
		assert(edge_list.size() != std::numeric_limits<vertex_id_t>::max());

		vertices.push_back(
			compact_vertex_t{.label = src_vert.label, .id = vertex_id, .edges = edge_list});

		iter += vertex_id_to_n_edges[vertex_id];
	}

	// 4: Copy edges over
	for (const auto& edge : input_edges)
	{
		const auto from = vertex_id_map[edge.from];
		const auto to = vertex_id_map[edge.to];
		assert(from != std::numeric_limits<vertex_id_t>::max());
		assert(to != std::numeric_limits<vertex_id_t>::max());

		// The spans are fixed size, so do some math with the number of vertices to figure out where
		// we should put the edges:
		auto& from_vert = vertices[from];
		from_vert.edges[from_vert.edges.size() - vertex_id_to_n_edges[from_vert.id]--] =
			edge_t{.from = from, .to = to, .label = edge.label, .id = edge.id};
		auto& to_vert = vertices[to];
		to_vert.edges[to_vert.edges.size() - vertex_id_to_n_edges[to_vert.id]--] =
			edge_t{.from = to, .to = from, .label = edge.label, .id = edge.id};
	}
}

// TODO:
[[nodiscard]] auto preprocess(std::vector<parsed_input_graph_t>&& graphs, std::size_t min_freq)
	-> std::vector<compact_graph_t>
{
	const auto frequent_vertex_labels = find_frequent_vertex_labels(graphs, min_freq);

	const auto frequent_edge_labels =
		find_frequent_edge_labels(graphs, frequent_vertex_labels, min_freq);

	// Reusable scratch memory
	std::vector<vertex_id_t> vertex_id_to_n_edges;
	std::vector<vertex_id_t> vertex_id_map;
	std::vector<edge_t> frequent_edges;

	std::vector<compact_graph_t> result;

	for (auto&& input : graphs)
	{
		for (edge_id_t i = 0; i < input.edges.size(); ++i)
		{
			const auto& edge = input.edges[i];
			const auto from_label = input.vertices[edge.from].label;
			const auto to_label = input.vertices[edge.to].label;
			const combined_edge_label combo{from_label, edge.label, to_label};

			if (frequent_edge_labels.contains(combo))
			{
				assert(frequent_vertex_labels.contains(from_label));
				assert(frequent_vertex_labels.contains(to_label));
				frequent_edges.push_back(
					edge_t{.from = edge.from, .to = edge.to, .label = edge.label, .id = i});
			}
		}

		// Save memory as we go, force deallocation here
		input.edges = {};

		if (!frequent_edges.empty())
		{
			result.push_back(
				compact_graph_t{input, frequent_edges, vertex_id_to_n_edges, vertex_id_map});
		}

		input.vertices = {};

		frequent_edges.clear();
	}

	return result;
}

} // namespace spang
