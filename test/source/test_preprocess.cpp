#include <spang/parser.hpp>
#include <spang/preprocess.hpp>

#include <catch2/catch_test_macros.hpp>

#include <fstream>

using spang::input_parser;
using spang::preprocess;

TEST_CASE("preprocess input")
{
	input_parser parser;
	{
		std::ifstream infile("data/data1.txt");

		parser.read(infile);
	}
	const auto& data = parser.get_graphs();

	SECTION("minfreq = 1")
	{
		auto data_copy = data;
		const auto result = preprocess(std::move(data_copy), 1);
		/*
		const auto [freq_vertex_label_occurrences, freq_edge_label_occurrences] =
			find_frequent_label_occurrences(data, 1);
		const auto [freq_vertex_labels, freq_edge_labels] = find_frequent_labels(data, 1);

		REQUIRE(freq_vertex_label_occurrences.size() == 3);
		REQUIRE(freq_vertex_labels.size() == 3);
		CHECK(freq_vertex_label_occurrences.at(0) == std::vector<spang::graph_id_t>{0, 2, 4});
		CHECK(freq_vertex_labels.at(0) == 3);
		CHECK(freq_vertex_label_occurrences.at(1) == std::vector<spang::graph_id_t>{0, 1, 2, 3, 4});
		CHECK(freq_vertex_labels.at(1) == 5);
		CHECK(freq_vertex_label_occurrences.at(2) == std::vector<spang::graph_id_t>{1, 2, 3, 4});
		CHECK(freq_vertex_labels.at(2) == 4);

		REQUIRE(freq_edge_label_occurrences.size() == 5);
		REQUIRE(freq_edge_labels.size() == 5);
		CHECK(freq_edge_label_occurrences.at(4) == std::vector<spang::graph_id_t>{1, 2, 3, 4});
		CHECK(freq_edge_labels.at(4) == 4);
		CHECK(freq_edge_label_occurrences.at(5) == std::vector<spang::graph_id_t>{1, 2, 3, 4});
		CHECK(freq_edge_labels.at(5) == 4);
		CHECK(freq_edge_label_occurrences.at(6) == std::vector<spang::graph_id_t>{0, 1, 2, 3, 4});
		CHECK(freq_edge_labels.at(6) == 5);
		CHECK(freq_edge_label_occurrences.at(7) == std::vector<spang::graph_id_t>{0, 1});
		CHECK(freq_edge_labels.at(7) == 2);
		CHECK(freq_edge_label_occurrences.at(8) == std::vector<spang::graph_id_t>{0, 1});
		CHECK(freq_edge_labels.at(8) == 2);
		*/
	}
	/*
	SECTION("minfreq = 2")
	{
		const auto [freq_vertex_label_occurrences, freq_edge_label_occurrences] =
			find_frequent_label_occurrences(data, 2);
		const auto [freq_vertex_labels, freq_edge_labels] = find_frequent_labels(data, 2);

		REQUIRE(freq_vertex_label_occurrences.size() == 3);
		REQUIRE(freq_vertex_labels.size() == 3);
		CHECK(freq_vertex_label_occurrences.at(0) == std::vector<spang::graph_id_t>{0, 2, 4});
		CHECK(freq_vertex_labels.at(0) == 3);
		CHECK(freq_vertex_label_occurrences.at(1) == std::vector<spang::graph_id_t>{0, 1, 2, 3, 4});
		CHECK(freq_vertex_labels.at(1) == 5);
		CHECK(freq_vertex_label_occurrences.at(2) == std::vector<spang::graph_id_t>{1, 2, 3, 4});
		CHECK(freq_vertex_labels.at(2) == 4);

		REQUIRE(freq_edge_label_occurrences.size() == 5);
		REQUIRE(freq_edge_labels.size() == 5);
		CHECK(freq_edge_label_occurrences.at(4) == std::vector<spang::graph_id_t>{1, 2, 3, 4});
		CHECK(freq_edge_labels.at(4) == 4);
		CHECK(freq_edge_label_occurrences.at(5) == std::vector<spang::graph_id_t>{1, 2, 3, 4});
		CHECK(freq_edge_labels.at(5) == 4);
		CHECK(freq_edge_label_occurrences.at(6) == std::vector<spang::graph_id_t>{0, 1, 2, 3, 4});
		CHECK(freq_edge_labels.at(6) == 5);
		CHECK(freq_edge_label_occurrences.at(7) == std::vector<spang::graph_id_t>{0, 1});
		CHECK(freq_edge_labels.at(7) == 2);
		CHECK(freq_edge_label_occurrences.at(8) == std::vector<spang::graph_id_t>{0, 1});
		CHECK(freq_edge_labels.at(8) == 2);
	}

	SECTION("minfreq = 3")
	{
		const auto [freq_vertex_label_occurrences, freq_edge_label_occurrences] =
			find_frequent_label_occurrences(data, 3);
		const auto [freq_vertex_labels, freq_edge_labels] = find_frequent_labels(data, 3);

		REQUIRE(freq_vertex_label_occurrences.size() == 3);
		REQUIRE(freq_vertex_labels.size() == 3);
		CHECK(freq_vertex_label_occurrences.at(0) == std::vector<spang::graph_id_t>{0, 2, 4});
		CHECK(freq_vertex_labels.at(0) == 3);
		CHECK(freq_vertex_label_occurrences.at(1) == std::vector<spang::graph_id_t>{0, 1, 2, 3, 4});
		CHECK(freq_vertex_labels.at(1) == 5);
		CHECK(freq_vertex_label_occurrences.at(2) == std::vector<spang::graph_id_t>{1, 2, 3, 4});
		CHECK(freq_vertex_labels.at(2) == 4);

		REQUIRE(freq_edge_label_occurrences.size() == 3);
		REQUIRE(freq_edge_labels.size() == 3);
		CHECK(freq_edge_label_occurrences.at(4) == std::vector<spang::graph_id_t>{1, 2, 3, 4});
		CHECK(freq_edge_labels.at(4) == 4);
		CHECK(freq_edge_label_occurrences.at(5) == std::vector<spang::graph_id_t>{1, 2, 3, 4});
		CHECK(freq_edge_labels.at(5) == 4);
		CHECK(freq_edge_label_occurrences.at(6) == std::vector<spang::graph_id_t>{0, 1, 2, 3, 4});
		CHECK(freq_edge_labels.at(6) == 5);
	}

	SECTION("minfreq = 4")
	{
		const auto [freq_vertex_label_occurrences, freq_edge_label_occurrences] =
			find_frequent_label_occurrences(data, 4);
		const auto [freq_vertex_labels, freq_edge_labels] = find_frequent_labels(data, 4);

		REQUIRE(freq_vertex_label_occurrences.size() == 2);
		REQUIRE(freq_vertex_labels.size() == 2);
		CHECK(freq_vertex_label_occurrences.at(1) == std::vector<spang::graph_id_t>{0, 1, 2, 3, 4});
		CHECK(freq_vertex_labels.at(1) == 5);
		CHECK(freq_vertex_label_occurrences.at(2) == std::vector<spang::graph_id_t>{1, 2, 3, 4});
		CHECK(freq_vertex_labels.at(2) == 4);

		REQUIRE(freq_edge_label_occurrences.size() == 3);
		REQUIRE(freq_edge_labels.size() == 3);
		CHECK(freq_edge_label_occurrences.at(4) == std::vector<spang::graph_id_t>{1, 2, 3, 4});
		CHECK(freq_edge_labels.at(4) == 4);
		CHECK(freq_edge_label_occurrences.at(5) == std::vector<spang::graph_id_t>{1, 2, 3, 4});
		CHECK(freq_edge_labels.at(5) == 4);
		CHECK(freq_edge_label_occurrences.at(6) == std::vector<spang::graph_id_t>{0, 1, 2, 3, 4});
		CHECK(freq_edge_labels.at(6) == 5);
	}

	SECTION("minfreq = 5")
	{
		const auto [freq_vertex_label_occurrences, freq_edge_label_occurrences] =
			find_frequent_label_occurrences(data, 5);
		const auto [freq_vertex_labels, freq_edge_labels] = find_frequent_labels(data, 5);

		REQUIRE(freq_vertex_label_occurrences.size() == 1);
		REQUIRE(freq_vertex_labels.size() == 1);
		CHECK(freq_vertex_label_occurrences.at(1) == std::vector<spang::graph_id_t>{0, 1, 2, 3, 4});
		CHECK(freq_vertex_labels.at(1) == 5);

		REQUIRE(freq_edge_label_occurrences.size() == 1);
		REQUIRE(freq_edge_labels.size() == 1);
		CHECK(freq_edge_label_occurrences.at(6) == std::vector<spang::graph_id_t>{0, 1, 2, 3, 4});
		CHECK(freq_edge_labels.at(6) == 5);
	}

	SECTION("minfreq = 6")
	{
		// Sanity check, there are only 5 graphs so nothing will be frequent here.
		const auto [freq_vertex_label_occurrences, freq_edge_label_occurrences] =
			find_frequent_label_occurrences(data, 6);
		const auto [freq_vertex_labels, freq_edge_labels] = find_frequent_labels(data, 6);

		REQUIRE(freq_vertex_label_occurrences.empty());
		REQUIRE(freq_vertex_labels.empty());
		REQUIRE(freq_edge_label_occurrences.empty());
		REQUIRE(freq_edge_labels.empty());
	}
	*/
}
