#include <spang/graph.hpp>
#include <spang/logger.hpp>
#include <spang/parser.hpp>

#include <sstream>
#include <string>

namespace spang
{

void input_parser::read(std::istream& stream)
{
	// This overall could be optimized, but currently this implementation
	// is aimed at simplicity with reasonable error reporting.
	std::string buffer;
	std::size_t line_no = 0;
	while (std::getline(stream, buffer))
	{
		std::istringstream line(buffer);

		char line_type;

		if (!(line >> line_type))
			// Empty line, ignore.
			continue;

		switch (line_type)
		{
		case 't':
		{
			char pound;
			graph_id_t id;
			if (!(line >> pound && pound == '#' && line >> id))
				log_error("line ", line_no, ", expected \"t # <id>\"");

			graphs.emplace_back(id);
			break;
		}
		case 'v':
		{
			vertex_id_t id;
			vertex_label_t label;
			if (!(line >> id >> label))
				log_error("line ", line_no, ", expected \"v <id> <label>\"");

			graphs.back().vertices.emplace_back(id, label);
			break;
		}
		case 'e':
		{
			vertex_id_t from, to;
			edge_label_t label;
			if (!(line >> from >> to >> label))
				log_error("line ", line_no, ", expected \"e <from_id> <to_id> <label>\"");

			// TODO: Proper error reporting for this
			assert(graphs.back().vertices.size() > static_cast<std::size_t>(from));
			assert(graphs.back().vertices.size() > static_cast<std::size_t>(to));
			graphs.back().edges.emplace_back(from, to, label);
			break;
		}
		case '#':
		{
			// Comment, do nothing.
			break;
		}
		default:
		{
			log_error("invalid token '", line_type, "' on line ", line_no,
			          ", expected t, v, or e.");
		}
		}
	}
}

void output_parser::read(std::istream& stream)
{
	// This overall could be optimized, but currently this implementation
	// is aimed at simplicity with reasonable error reporting.
	std::string buffer;
	std::size_t line_no = 0;
	parsed_output_graph_t current;
	bool first = true;
	while (std::getline(stream, buffer))
	{
		std::istringstream line(buffer);

		char line_type;

		if (!(line >> line_type))
			// Empty line, ignore.
			continue;

		switch (line_type)
		{
		case 't':
		{
			char pound, asterisk;
			graph_id_t id;
			std::size_t supp;
			if (!(line >> pound && pound == '#' && line >> id >> asterisk && asterisk == '*' &&
			      line >> supp))
			{
				log_error("line ", line_no, ", expected \"t # <id> * <support>\"");
				return;
			}

			if (first)
			{
				first = false;
			}
			else
			{
				// Insert the current graph
				graphs.emplace(std::move(current));

				// The moved-from vectors are guaranteed to
				// be empty(), no need to clear them.
			}

			current.id = id;
			current.support.reserve(supp);
			break;
		}
		case 'v':
		{
			vertex_id_t id;
			vertex_label_t label;
			if (!(line >> id >> label))
				log_error("line ", line_no, ", expected \"v <id> <label>\"");

			current.vertices.emplace_back(id, label);
			break;
		}
		case 'e':
		{
			vertex_id_t from, to;
			edge_label_t label;
			if (!(line >> from >> to >> label))
				log_error("line ", line_no, ", expected \"e <from_id> <to_id> <label>\"");

			current.edges.emplace_back(from, to, label);
			break;
		}
		case 'x':
		{
			char colon;
			graph_id_t temp;
			if (!(line >> colon && colon == ':'))
				log_error("line ", line_no, ", expected \"x: <support list>\"");

			while (line >> temp)
				current.support.emplace_back(temp);
			break;
		}
		default:
			log_error("invalid token '", line_type, "' on line ", line_no,
			          ", expected t, v, e, or x.");
		}
	}

	// Add the last graph.
	graphs.emplace(std::move(current));
}

} // namespace spang
