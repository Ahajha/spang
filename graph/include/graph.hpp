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
	int n_edges;
	std::vector<vertex_t> vertices;
};

}
