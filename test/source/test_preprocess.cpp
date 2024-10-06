#include <spang/parser.hpp>
#include <spang/preprocess.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <fstream>

using spang::edge_t;
using spang::input_parser;
using spang::preprocess;

// Reverse an edge
[[nodiscard]] constexpr static edge_t rev(const edge_t& e)
{
	return edge_t{.from = e.to, .to = e.from, .label = e.label, .id = e.id};
}

/*
Example data (data1) frequencies/occurrences of labels:
Vertex labels:
0: 1, 3, 5
1: 1, 2, 3, 4, 5
2: 2, 3, 4, 5

1-edge graphs (combo vlabel + elabel + vlabel):
0-5-1: 3, 5
0-5-2: 3, 5
0-7-1: 1(x3)
0-8-0: 1
1-4-1: 2, 4, 5
1-4-2: 3
1-5-2: 2, 3, 4, 5
1-6-1: 1, 3
1-6-2: 2, 4, 5(x2)
1-7-2: 2
1-8-2: 2
*/

// TODO: Also add vertex label tests
// Some results will change:
// No more empty vertices
// Where there were, indexes will change, so can't use a single list of edges
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
		REQUIRE(result.size() == 5);

		constexpr edge_t g1e1{.from = 0, .to = 1, .label = 8, .id = 0};
		constexpr edge_t g1e2{.from = 0, .to = 2, .label = 7, .id = 1};
		constexpr edge_t g1e3{.from = 0, .to = 3, .label = 7, .id = 2};
		constexpr edge_t g1e4{.from = 1, .to = 3, .label = 7, .id = 3};
		constexpr edge_t g1e5{.from = 2, .to = 3, .label = 6, .id = 4};

		REQUIRE(result[0].vertices.size() == 4);
		CHECK(std::ranges::equal(result[0].vertices[0].edges, std::array{g1e1, g1e2, g1e3}));
		CHECK(std::ranges::equal(result[0].vertices[1].edges, std::array{rev(g1e1), g1e4}));
		CHECK(std::ranges::equal(result[0].vertices[2].edges, std::array{rev(g1e2), g1e5}));
		CHECK(std::ranges::equal(result[0].vertices[3].edges,
		                         std::array{rev(g1e3), rev(g1e4), rev(g1e5)}));

		constexpr edge_t g2e1{.from = 0, .to = 1, .label = 7, .id = 0};
		constexpr edge_t g2e2{.from = 0, .to = 2, .label = 6, .id = 1};
		constexpr edge_t g2e3{.from = 0, .to = 3, .label = 4, .id = 2};
		constexpr edge_t g2e4{.from = 1, .to = 3, .label = 8, .id = 3};
		constexpr edge_t g2e5{.from = 2, .to = 3, .label = 5, .id = 4};

		REQUIRE(result[1].vertices.size() == 4);
		CHECK(std::ranges::equal(result[1].vertices[0].edges, std::array{g2e1, g2e2, g2e3}));
		CHECK(std::ranges::equal(result[1].vertices[1].edges, std::array{rev(g2e1), g2e4}));
		CHECK(std::ranges::equal(result[1].vertices[2].edges, std::array{rev(g2e2), g2e5}));
		CHECK(std::ranges::equal(result[1].vertices[3].edges,
		                         std::array{rev(g2e3), rev(g2e4), rev(g2e5)}));

		constexpr edge_t g3e1{.from = 0, .to = 1, .label = 5, .id = 0};
		constexpr edge_t g3e2{.from = 0, .to = 2, .label = 5, .id = 1};
		constexpr edge_t g3e3{.from = 1, .to = 2, .label = 5, .id = 2};
		constexpr edge_t g3e4{.from = 1, .to = 3, .label = 6, .id = 3};
		constexpr edge_t g3e5{.from = 2, .to = 3, .label = 4, .id = 4};

		REQUIRE(result[2].vertices.size() == 4);
		CHECK(std::ranges::equal(result[2].vertices[0].edges, std::array{g3e1, g3e2}));
		CHECK(std::ranges::equal(result[2].vertices[1].edges, std::array{rev(g3e1), g3e3, g3e4}));
		CHECK(std::ranges::equal(result[2].vertices[2].edges,
		                         std::array{rev(g3e2), rev(g3e3), g3e5}));
		CHECK(std::ranges::equal(result[2].vertices[3].edges, std::array{rev(g3e4), rev(g3e5)}));

		constexpr edge_t g4e1{.from = 0, .to = 1, .label = 4, .id = 0};
		constexpr edge_t g4e2{.from = 0, .to = 2, .label = 5, .id = 1};
		constexpr edge_t g4e3{.from = 1, .to = 2, .label = 6, .id = 2};

		REQUIRE(result[3].vertices.size() == 3);
		CHECK(std::ranges::equal(result[3].vertices[0].edges, std::array{g4e1, g4e2}));
		CHECK(std::ranges::equal(result[3].vertices[1].edges, std::array{rev(g4e1), g4e3}));
		CHECK(std::ranges::equal(result[3].vertices[2].edges, std::array{rev(g4e2), rev(g4e3)}));

		constexpr edge_t g5e1{.from = 0, .to = 1, .label = 5, .id = 0};
		constexpr edge_t g5e2{.from = 0, .to = 2, .label = 4, .id = 1};
		constexpr edge_t g5e3{.from = 0, .to = 3, .label = 5, .id = 2};
		constexpr edge_t g5e4{.from = 1, .to = 3, .label = 5, .id = 3};
		constexpr edge_t g5e5{.from = 2, .to = 3, .label = 6, .id = 4};
		constexpr edge_t g5e6{.from = 3, .to = 4, .label = 6, .id = 5};

		REQUIRE(result[4].vertices.size() == 5);
		CHECK(std::ranges::equal(result[4].vertices[0].edges, std::array{g5e1, g5e2, g5e3}));
		CHECK(std::ranges::equal(result[4].vertices[1].edges, std::array{rev(g5e1), g5e4}));
		CHECK(std::ranges::equal(result[4].vertices[2].edges, std::array{rev(g5e2), g5e5}));
		CHECK(std::ranges::equal(result[4].vertices[3].edges,
		                         std::array{rev(g5e3), rev(g5e4), rev(g5e5), g5e6}));
		CHECK(std::ranges::equal(result[4].vertices[4].edges, std::array{rev(g5e6)}));
	}

	SECTION("minfreq = 2")
	{
		auto data_copy = data;
		const auto result = preprocess(std::move(data_copy), 2);
		REQUIRE(result.size() == 5);

		// pruned: v0, v1, g1e1, g1e2, g1e3,
		// vertex map: 2 -> 0, 3 -> 1
		constexpr edge_t g1e5{.from = 0, .to = 1, .label = 6, .id = 4};

		REQUIRE(result[0].vertices.size() == 2);
		CHECK(std::ranges::equal(result[0].vertices[0].edges, std::array{g1e5}));
		CHECK(std::ranges::equal(result[0].vertices[1].edges, std::array{rev(g1e5)}));

		// pruned: v1, g2e1, g2e4
		// vertex map: 2 -> 1, 3 -> 2
		constexpr edge_t g2e2{.from = 0, .to = 1, .label = 6, .id = 1};
		constexpr edge_t g2e3{.from = 0, .to = 2, .label = 4, .id = 2};
		constexpr edge_t g2e5{.from = 1, .to = 2, .label = 5, .id = 4};

		REQUIRE(result[1].vertices.size() == 3);
		CHECK(std::ranges::equal(result[1].vertices[0].edges, std::array{g2e2, g2e3}));
		CHECK(std::ranges::equal(result[1].vertices[1].edges, std::array{rev(g2e2), g2e5}));
		CHECK(std::ranges::equal(result[1].vertices[2].edges, std::array{rev(g2e3), rev(g2e5)}));

		constexpr edge_t g3e1{.from = 0, .to = 1, .label = 5, .id = 0};
		constexpr edge_t g3e2{.from = 0, .to = 2, .label = 5, .id = 1};
		constexpr edge_t g3e3{.from = 1, .to = 2, .label = 5, .id = 2};
		constexpr edge_t g3e4{.from = 1, .to = 3, .label = 6, .id = 3};

		// pruned: g3e5
		REQUIRE(result[2].vertices.size() == 4);
		CHECK(std::ranges::equal(result[2].vertices[0].edges, std::array{g3e1, g3e2}));
		CHECK(std::ranges::equal(result[2].vertices[1].edges, std::array{rev(g3e1), g3e3, g3e4}));
		CHECK(std::ranges::equal(result[2].vertices[2].edges, std::array{rev(g3e2), rev(g3e3)}));
		CHECK(std::ranges::equal(result[2].vertices[3].edges, std::array{rev(g3e4)}));

		// pruned: none
		constexpr edge_t g4e1{.from = 0, .to = 1, .label = 4, .id = 0};
		constexpr edge_t g4e2{.from = 0, .to = 2, .label = 5, .id = 1};
		constexpr edge_t g4e3{.from = 1, .to = 2, .label = 6, .id = 2};

		REQUIRE(result[3].vertices.size() == 3);
		CHECK(std::ranges::equal(result[3].vertices[0].edges, std::array{g4e1, g4e2}));
		CHECK(std::ranges::equal(result[3].vertices[1].edges, std::array{rev(g4e1), g4e3}));
		CHECK(std::ranges::equal(result[3].vertices[2].edges, std::array{rev(g4e2), rev(g4e3)}));

		// pruned: none
		constexpr edge_t g5e1{.from = 0, .to = 1, .label = 5, .id = 0};
		constexpr edge_t g5e2{.from = 0, .to = 2, .label = 4, .id = 1};
		constexpr edge_t g5e3{.from = 0, .to = 3, .label = 5, .id = 2};
		constexpr edge_t g5e4{.from = 1, .to = 3, .label = 5, .id = 3};
		constexpr edge_t g5e5{.from = 2, .to = 3, .label = 6, .id = 4};
		constexpr edge_t g5e6{.from = 3, .to = 4, .label = 6, .id = 5};

		REQUIRE(result[4].vertices.size() == 5);
		CHECK(std::ranges::equal(result[4].vertices[0].edges, std::array{g5e1, g5e2, g5e3}));
		CHECK(std::ranges::equal(result[4].vertices[1].edges, std::array{rev(g5e1), g5e4}));
		CHECK(std::ranges::equal(result[4].vertices[2].edges, std::array{rev(g5e2), g5e5}));
		CHECK(std::ranges::equal(result[4].vertices[3].edges,
		                         std::array{rev(g5e3), rev(g5e4), rev(g5e5), g5e6}));
		CHECK(std::ranges::equal(result[4].vertices[4].edges, std::array{rev(g5e6)}));
	}

	constexpr edge_t g3e3{.from = 0, .to = 1, .label = 5, .id = 2};

	SECTION("minfreq = 3")
	{
		auto data_copy = data;
		const auto result = preprocess(std::move(data_copy), 3);
		REQUIRE(result.size() == 4);

		// graph 1: completely removed

		// pruned: g2e1, g2e4
		// pruned: v1, g2e1, g2e4
		// vertex map: 2 -> 1, 3 -> 2
		constexpr edge_t g2e2{.from = 0, .to = 1, .label = 6, .id = 1};
		constexpr edge_t g2e3{.from = 0, .to = 2, .label = 4, .id = 2};
		constexpr edge_t g2e5{.from = 1, .to = 2, .label = 5, .id = 4};

		REQUIRE(result[0].vertices.size() == 3);
		CHECK(std::ranges::equal(result[0].vertices[0].edges, std::array{g2e2, g2e3}));
		CHECK(std::ranges::equal(result[0].vertices[1].edges, std::array{rev(g2e2), g2e5}));
		CHECK(std::ranges::equal(result[0].vertices[2].edges, std::array{rev(g2e3), rev(g2e5)}));

		// pruned: v0, v3, g3e1, g3e2, g3e4, g3e5
		// vertex map: 1 -> 0, 2 -> 1
		REQUIRE(result[1].vertices.size() == 2);
		CHECK(std::ranges::equal(result[1].vertices[0].edges, std::array{g3e3}));
		CHECK(std::ranges::equal(result[1].vertices[1].edges, std::array{rev(g3e3)}));

		// pruned: none
		constexpr edge_t g4e1{.from = 0, .to = 1, .label = 4, .id = 0};
		constexpr edge_t g4e2{.from = 0, .to = 2, .label = 5, .id = 1};
		constexpr edge_t g4e3{.from = 1, .to = 2, .label = 6, .id = 2};

		REQUIRE(result[2].vertices.size() == 3);
		CHECK(std::ranges::equal(result[2].vertices[0].edges, std::array{g4e1, g4e2}));
		CHECK(std::ranges::equal(result[2].vertices[1].edges, std::array{rev(g4e1), g4e3}));
		CHECK(std::ranges::equal(result[2].vertices[2].edges, std::array{rev(g4e2), rev(g4e3)}));

		// pruned: v1, g5e1, g5e4
		// vertex map: 2 -> 1, 3 -> 2, 4 -> 3
		constexpr edge_t g5e2{.from = 0, .to = 1, .label = 4, .id = 1};
		constexpr edge_t g5e3{.from = 0, .to = 2, .label = 5, .id = 2};
		constexpr edge_t g5e5{.from = 1, .to = 2, .label = 6, .id = 4};
		constexpr edge_t g5e6{.from = 2, .to = 3, .label = 6, .id = 5};

		REQUIRE(result[3].vertices.size() == 4);
		CHECK(std::ranges::equal(result[3].vertices[0].edges, std::array{g5e2, g5e3}));
		CHECK(std::ranges::equal(result[3].vertices[1].edges, std::array{rev(g5e2), g5e5}));
		CHECK(std::ranges::equal(result[3].vertices[2].edges,
		                         std::array{rev(g5e3), rev(g5e5), g5e6}));
		CHECK(std::ranges::equal(result[3].vertices[3].edges, std::array{rev(g5e6)}));
	}

	SECTION("minfreq = 4")
	{
		auto data_copy = data;
		const auto result = preprocess(std::move(data_copy), 4);
		REQUIRE(result.size() == 4);

		// graph 1 pruned entirely

		// pruned: all except g2e5
		constexpr edge_t g2e5{.from = 0, .to = 1, .label = 5, .id = 4};

		REQUIRE(result[0].vertices.size() == 2);
		CHECK(std::ranges::equal(result[0].vertices[0].edges, std::array{g2e5}));
		CHECK(std::ranges::equal(result[0].vertices[1].edges, std::array{rev(g2e5)}));

		// pruned: all except g3e3
		constexpr edge_t g3e3_final{.from = 0, .to = 1, .label = 5, .id = 2};

		REQUIRE(result[1].vertices.size() == 2);
		CHECK(std::ranges::equal(result[1].vertices[0].edges, std::array{g3e3_final}));
		CHECK(std::ranges::equal(result[1].vertices[1].edges, std::array{rev(g3e3_final)}));

		// pruned: g4e1, g4e3
		constexpr edge_t g4e2{.from = 0, .to = 1, .label = 5, .id = 1};

		REQUIRE(result[2].vertices.size() == 2);
		CHECK(std::ranges::equal(result[2].vertices[0].edges, std::array{g4e2}));
		CHECK(std::ranges::equal(result[2].vertices[1].edges, std::array{rev(g4e2)}));

		// pruned: all but g5e3, also v1
		constexpr edge_t g5e3{.from = 0, .to = 1, .label = 5, .id = 2};

		REQUIRE(result[3].vertices.size() == 2);
		CHECK(std::ranges::equal(result[3].vertices[0].edges, std::array{g5e3}));
		CHECK(std::ranges::equal(result[3].vertices[1].edges, std::array{rev(g5e3)}));
	}
}
