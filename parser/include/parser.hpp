#pragma once

#include <graph.hpp>
#include <iostream>
#include <set>
#include <vector>

namespace spang
{

struct parsed_edge_t
{
	vertex_id_t from, to;
	edge_label_t label;

	bool operator==(const parsed_edge_t&) const = default;
	auto operator<=>(const parsed_edge_t&) const = default;
};

struct parsed_vertex_t
{
	vertex_id_t id;
	vertex_label_t label;

	bool operator==(const parsed_vertex_t&) const = default;
	auto operator<=>(const parsed_vertex_t&) const = default;
};

struct parsed_input_graph_t
{
	graph_id_t id;
	std::vector<parsed_vertex_t> vertices;
	std::vector<parsed_edge_t> edges;
};

//! This class is not intended to be used directly, but exists so that the comparison
//! operators can be defaulted while ignoring the graph ID.
struct parsed_output_graph_base_t
{
	std::vector<parsed_vertex_t> vertices;
	std::vector<parsed_edge_t> edges;
	std::vector<graph_id_t> support;

	bool operator==(const parsed_output_graph_base_t&) const = default;
	auto operator<=>(const parsed_output_graph_base_t&) const = default;
};

struct parsed_output_graph_t : parsed_output_graph_base_t
{
	graph_id_t id;
};

/*!
Class for parsing input files.
*/
class input_parser
{
  public:
	void read(std::istream& stream);

	const auto& get_graphs() const { return graphs; }

  private:
	std::vector<parsed_input_graph_t> graphs;
};

/*!
Class for re-parsing output files. Intended to be used
to compare output files of different implementations
to ensure correctness.
*/
class output_parser
{
  public:
	void read(std::istream& stream);

	const auto& get_graphs() const { return graphs; }

  private:
	// The intended use case is to simply compare the output from two different
	// parsers, so this makes that comparison simple.
	std::set<parsed_output_graph_t> graphs;
};

} // namespace spang
