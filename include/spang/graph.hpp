#pragma once

#include <vector>

namespace spang
{

using graph_id_t = int;
using vertex_id_t = int;
using edge_id_t = int;
using vertex_label_t = int;
using edge_label_t = int;

struct edge_t
{
	vertex_id_t to, from;
	edge_label_t label;
	edge_id_t id;
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
	int n_edges = 0;
	std::vector<vertex_t> vertices;

	//! Adds an undirected edge to the graph.
	void add_edge(vertex_id_t from_id, edge_label_t edge_label, vertex_id_t to_id)
	{
		const auto edge_id = static_cast<edge_id_t>(n_edges);
		vertices[to_id].edges.emplace_back(to_id, from_id, edge_label, edge_id);
		vertices[from_id].edges.emplace_back(from_id, to_id, edge_label, edge_id);
		++n_edges;
	}
};

} // namespace spang
