#pragma once

#include <cassert>
#include <cstdint>
#include <vector>

namespace spang
{

using graph_id_t = int;
using vertex_id_t = std::uint16_t;
using edge_id_t = std::uint16_t;
using vertex_label_t = int;
using edge_label_t = int;

struct edge_t
{
	// Note: These are indexes of the vertices, which
	// might not necessarily be the ID of that vertex.
	vertex_id_t from, to;
	edge_label_t label;
	edge_id_t id;

	[[nodiscard]] constexpr bool operator==(const edge_t&) const = default;
};

struct vertex_t
{
	vertex_label_t label;
	vertex_id_t id;
	std::vector<edge_t> edges;
};

struct graph_t
{
	graph_id_t id;
	std::uint32_t n_edges = 0;
	std::vector<vertex_t> vertices;

	//! Adds an undirected edge to the graph.
	void add_edge(vertex_id_t from_id, edge_label_t edge_label, vertex_id_t to_id)
	{
		assert(to_id < vertices.size());
		assert(from_id < vertices.size());
		const auto edge_id = static_cast<edge_id_t>(n_edges);
		vertices[to_id].edges.push_back(
			edge_t{.from = to_id, .to = from_id, .label = edge_label, .id = edge_id});
		vertices[from_id].edges.push_back(
			edge_t{.from = from_id, .to = to_id, .label = edge_label, .id = edge_id});
		++n_edges;
	}
};

} // namespace spang
