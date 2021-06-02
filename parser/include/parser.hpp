#pragma once

#include <graph.hpp>
#include <vector>
#include <set>
#include <iostream>

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

struct parsed_output_graph_t
{
	graph_id_t id;
	std::vector<parsed_vertex_t> vertices;
	std::vector<parsed_edge_t> edges;
	std::vector<graph_id_t> support;
	
	// These cannot be defaulted, as we need to ignore the id.
	auto operator<=>(const parsed_output_graph_t& other) const
	{
		const auto comp1 = vertices <=> other.vertices;
		if (comp1 != 0) return comp1;
		
		const auto comp2 = edges <=> other.edges;
		if (comp2 != 0) return comp2;
		
		const auto comp3 = support <=> other.support;
		return comp3;
	}
	
	bool operator==(const parsed_output_graph_t& other) const
	{
		return vertices == other.vertices
		    && edges == other.edges
		    && support == other.support;
	}
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

}
