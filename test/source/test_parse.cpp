#include <spang/parser.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <fstream>
#include <vector>

TEST_CASE("parse input")
{
	using spang::input_parser;
	using spang::parsed_edge_t;

	input_parser parser;
	{
		std::ifstream infile("test/data/Chemical_340.txt");

		parser.read(infile);
	}
	const auto& data = parser.get_graphs();

	REQUIRE(data.size() == 340);
	for (std::size_t i = 0; i < data.size(); ++i)
	{
		CHECK(data[i].id == static_cast<spang::graph_id_t>(i));
	}

	SECTION("Verify first graph")
	{
		const auto& first = data.front();
		const std::array labels{
			0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 0, 0, 2, 0, 0, 0, 0, 1, 1, 1, 3, 3, 4, 5, 5,
		};
		REQUIRE(labels.size() == first.vertices.size());
		for (std::size_t i = 0; i < labels.size(); ++i)
		{
			CHECK(first.vertices[i].id == i);
			CHECK(first.vertices[i].label == labels[i]);
		}
		const std::vector edges = {
			parsed_edge_t{0, 1, 3},   parsed_edge_t{1, 2, 3},   parsed_edge_t{2, 3, 3},
			parsed_edge_t{3, 4, 3},   parsed_edge_t{4, 5, 3},   parsed_edge_t{5, 0, 3},
			parsed_edge_t{0, 6, 0},   parsed_edge_t{1, 7, 0},   parsed_edge_t{4, 8, 0},
			parsed_edge_t{5, 9, 0},   parsed_edge_t{2, 10, 0},  parsed_edge_t{10, 11, 0},
			parsed_edge_t{11, 12, 3}, parsed_edge_t{12, 13, 0}, parsed_edge_t{13, 3, 0},
			parsed_edge_t{11, 14, 3}, parsed_edge_t{14, 15, 3}, parsed_edge_t{15, 16, 3},
			parsed_edge_t{16, 17, 3}, parsed_edge_t{17, 12, 3}, parsed_edge_t{14, 18, 0},
			parsed_edge_t{15, 19, 0}, parsed_edge_t{17, 20, 0}, parsed_edge_t{13, 21, 1},
			parsed_edge_t{10, 22, 1}, parsed_edge_t{16, 23, 0}, parsed_edge_t{23, 24, 0},
			parsed_edge_t{23, 25, 0},
		};
		CHECK(first.edges == edges);
	}

	SECTION("Verify (arbitrary) middle graph")
	{
		const auto& middle = data[100];
		const std::array labels{
			0, 0, 39, 39, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 9, 9, 1, 1, 1, 1, 22, 3, 3,
		};
		REQUIRE(labels.size() == middle.vertices.size());
		for (std::size_t i = 0; i < labels.size(); ++i)
		{
			CHECK(middle.vertices[i].id == i);
			CHECK(middle.vertices[i].label == labels[i]);
		}
		const std::vector edges = {
			parsed_edge_t{0, 1, 3},   parsed_edge_t{1, 2, 3},   parsed_edge_t{2, 3, 3},
			parsed_edge_t{3, 4, 3},   parsed_edge_t{4, 5, 3},   parsed_edge_t{5, 0, 3},
			parsed_edge_t{0, 6, 0},   parsed_edge_t{1, 7, 0},   parsed_edge_t{5, 8, 0},
			parsed_edge_t{2, 9, 3},   parsed_edge_t{9, 10, 3},  parsed_edge_t{10, 11, 3},
			parsed_edge_t{11, 12, 3}, parsed_edge_t{12, 3, 3},  parsed_edge_t{10, 13, 0},
			parsed_edge_t{11, 14, 0}, parsed_edge_t{4, 15, 0},  parsed_edge_t{12, 16, 0},
			parsed_edge_t{16, 15, 0}, parsed_edge_t{16, 17, 0}, parsed_edge_t{16, 18, 0},
			parsed_edge_t{15, 19, 0}, parsed_edge_t{15, 20, 0}, parsed_edge_t{9, 21, 0},
			parsed_edge_t{21, 22, 1}, parsed_edge_t{21, 23, 1},
		};
		CHECK(middle.edges == edges);
	}

	SECTION("Verify last graph")
	{
		const auto& last = data.back();
		const std::array labels{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
		};
		REQUIRE(labels.size() == last.vertices.size());
		for (std::size_t i = 0; i < labels.size(); ++i)
		{
			CHECK(last.vertices[i].id == i);
			CHECK(last.vertices[i].label == labels[i]);
		}
		const std::vector edges = {
			parsed_edge_t{0, 1, 3},  parsed_edge_t{1, 2, 3},   parsed_edge_t{2, 3, 3},
			parsed_edge_t{3, 4, 3},  parsed_edge_t{4, 5, 3},   parsed_edge_t{5, 0, 3},
			parsed_edge_t{6, 7, 3},  parsed_edge_t{7, 8, 3},   parsed_edge_t{8, 9, 3},
			parsed_edge_t{9, 10, 3}, parsed_edge_t{10, 11, 3}, parsed_edge_t{11, 6, 3},
			parsed_edge_t{3, 12, 0}, parsed_edge_t{12, 6, 0},  parsed_edge_t{4, 13, 0},
			parsed_edge_t{5, 14, 0}, parsed_edge_t{0, 15, 0},  parsed_edge_t{1, 16, 0},
			parsed_edge_t{2, 17, 0}, parsed_edge_t{7, 18, 0},  parsed_edge_t{10, 19, 0},
			parsed_edge_t{9, 20, 0}, parsed_edge_t{8, 21, 0},  parsed_edge_t{11, 22, 0},
		};
		CHECK(last.edges == edges);
	}
}

TEST_CASE("parse output")
{
	// TODO
}
