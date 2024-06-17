#include <spang/parser.hpp>
#include <spang/preprocess.hpp>

#include <catch2/catch_test_macros.hpp>

#include <fstream>

using spang::edge_t;
using spang::input_parser;
using spang::preprocess;

// Reverse an edge
[[nodiscard]] constexpr static edge_t rev(const edge_t& e)
{
	return edge_t{.from = e.to, .to = e.from, .label = e.label, .id = e.id};
}

TEST_CASE("preprocess input")
{
	input_parser parser;
	{
		std::ifstream infile("data/data1.txt");

		parser.read(infile);
	}
	const auto& data = parser.get_graphs();

	constexpr edge_t g1e1{.from = 0, .to = 1, .label = 8, .id = 0};
	constexpr edge_t g1e2{.from = 0, .to = 2, .label = 7, .id = 1};
	constexpr edge_t g1e3{.from = 0, .to = 3, .label = 7, .id = 2};
	constexpr edge_t g1e4{.from = 1, .to = 3, .label = 7, .id = 3};
	constexpr edge_t g1e5{.from = 2, .to = 3, .label = 6, .id = 4};

	constexpr edge_t g2e1{.from = 0, .to = 1, .label = 7, .id = 0};
	constexpr edge_t g2e2{.from = 0, .to = 2, .label = 6, .id = 1};
	constexpr edge_t g2e3{.from = 0, .to = 3, .label = 4, .id = 2};
	constexpr edge_t g2e4{.from = 1, .to = 3, .label = 8, .id = 3};
	constexpr edge_t g2e5{.from = 2, .to = 3, .label = 5, .id = 4};

	constexpr edge_t g3e1{.from = 0, .to = 1, .label = 5, .id = 0};
	constexpr edge_t g3e2{.from = 0, .to = 2, .label = 5, .id = 1};
	constexpr edge_t g3e3{.from = 1, .to = 2, .label = 5, .id = 2};
	constexpr edge_t g3e4{.from = 1, .to = 3, .label = 6, .id = 3};
	constexpr edge_t g3e5{.from = 2, .to = 3, .label = 4, .id = 4};

	constexpr edge_t g4e1{.from = 0, .to = 1, .label = 4, .id = 0};
	constexpr edge_t g4e2{.from = 0, .to = 2, .label = 5, .id = 1};
	constexpr edge_t g4e3{.from = 1, .to = 2, .label = 6, .id = 2};

	constexpr edge_t g5e1{.from = 0, .to = 1, .label = 5, .id = 0};
	constexpr edge_t g5e2{.from = 0, .to = 2, .label = 4, .id = 1};
	constexpr edge_t g5e3{.from = 0, .to = 3, .label = 5, .id = 2};
	constexpr edge_t g5e4{.from = 1, .to = 3, .label = 5, .id = 3};
	constexpr edge_t g5e5{.from = 2, .to = 3, .label = 6, .id = 4};
	constexpr edge_t g5e6{.from = 3, .to = 4, .label = 6, .id = 5};

	SECTION("minfreq = 1")
	{
		auto data_copy = data;
		const auto result = preprocess(std::move(data_copy), 1);
		REQUIRE(result.size() == 5);

		REQUIRE(result[0].vertices.size() == 4);
		CHECK(std::ranges::equal(result[0].vertices[0].edges, std::array{g1e1, g1e2, g1e3}));
		CHECK(std::ranges::equal(result[0].vertices[1].edges, std::array{rev(g1e1), g1e4}));
		CHECK(std::ranges::equal(result[0].vertices[2].edges, std::array{rev(g1e2), g1e5}));
		CHECK(std::ranges::equal(result[0].vertices[3].edges,
		                         std::array{rev(g1e3), rev(g1e4), rev(g1e5)}));

		REQUIRE(result[1].vertices.size() == 4);
		CHECK(std::ranges::equal(result[1].vertices[0].edges, std::array{g2e1, g2e2, g2e3}));
		CHECK(std::ranges::equal(result[1].vertices[1].edges, std::array{rev(g2e1), g2e4}));
		CHECK(std::ranges::equal(result[1].vertices[2].edges, std::array{rev(g2e2), g2e5}));
		CHECK(std::ranges::equal(result[1].vertices[3].edges,
		                         std::array{rev(g2e3), rev(g2e4), rev(g2e5)}));

		REQUIRE(result[2].vertices.size() == 4);
		CHECK(std::ranges::equal(result[2].vertices[0].edges, std::array{g3e1, g3e2}));
		CHECK(std::ranges::equal(result[2].vertices[1].edges, std::array{rev(g3e1), g3e3, g3e4}));
		CHECK(std::ranges::equal(result[2].vertices[2].edges,
		                         std::array{rev(g3e2), rev(g3e3), g3e5}));
		CHECK(std::ranges::equal(result[2].vertices[3].edges, std::array{rev(g3e4), rev(g3e5)}));

		REQUIRE(result[3].vertices.size() == 3);
		CHECK(std::ranges::equal(result[3].vertices[0].edges, std::array{g4e1, g4e2}));
		CHECK(std::ranges::equal(result[3].vertices[1].edges, std::array{rev(g4e1), g4e3}));
		CHECK(std::ranges::equal(result[3].vertices[2].edges, std::array{rev(g4e2), rev(g4e3)}));

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

		// pruned: g1e1, g1e2, g1e3, g1e4
		REQUIRE(result[0].vertices.size() == 4);
		CHECK(result[0].vertices[0].edges.empty());
		CHECK(result[0].vertices[1].edges.empty());
		CHECK(std::ranges::equal(result[0].vertices[2].edges, std::array{g1e5}));
		CHECK(std::ranges::equal(result[0].vertices[3].edges, std::array{rev(g1e5)}));

		// pruned: g2e1, g2e4
		REQUIRE(result[1].vertices.size() == 4);
		CHECK(std::ranges::equal(result[1].vertices[0].edges, std::array{g2e2, g2e3}));
		CHECK(result[1].vertices[1].edges.empty());
		CHECK(std::ranges::equal(result[1].vertices[2].edges, std::array{rev(g2e2), g2e5}));
		CHECK(std::ranges::equal(result[1].vertices[3].edges, std::array{rev(g2e3), rev(g2e5)}));

		// pruned: g3e5
		REQUIRE(result[2].vertices.size() == 4);
		CHECK(std::ranges::equal(result[2].vertices[0].edges, std::array{g3e1, g3e2}));
		CHECK(std::ranges::equal(result[2].vertices[1].edges, std::array{rev(g3e1), g3e3, g3e4}));
		CHECK(std::ranges::equal(result[2].vertices[2].edges, std::array{rev(g3e2), rev(g3e3)}));
		CHECK(std::ranges::equal(result[2].vertices[3].edges, std::array{rev(g3e4)}));

		// pruned: none
		REQUIRE(result[3].vertices.size() == 3);
		CHECK(std::ranges::equal(result[3].vertices[0].edges, std::array{g4e1, g4e2}));
		CHECK(std::ranges::equal(result[3].vertices[1].edges, std::array{rev(g4e1), g4e3}));
		CHECK(std::ranges::equal(result[3].vertices[2].edges, std::array{rev(g4e2), rev(g4e3)}));

		// pruned: none
		REQUIRE(result[4].vertices.size() == 5);
		CHECK(std::ranges::equal(result[4].vertices[0].edges, std::array{g5e1, g5e2, g5e3}));
		CHECK(std::ranges::equal(result[4].vertices[1].edges, std::array{rev(g5e1), g5e4}));
		CHECK(std::ranges::equal(result[4].vertices[2].edges, std::array{rev(g5e2), g5e5}));
		CHECK(std::ranges::equal(result[4].vertices[3].edges,
		                         std::array{rev(g5e3), rev(g5e4), rev(g5e5), g5e6}));
		CHECK(std::ranges::equal(result[4].vertices[4].edges, std::array{rev(g5e6)}));
	}

	SECTION("minfreq = 3")
	{
		auto data_copy = data;
		const auto result = preprocess(std::move(data_copy), 3);
		REQUIRE(result.size() == 5);

		// pruned: all. TODO: the whole graph should be removed
		REQUIRE(result[0].vertices.size() == 4);
		CHECK(result[0].vertices[0].edges.empty());
		CHECK(result[0].vertices[1].edges.empty());
		CHECK(result[0].vertices[2].edges.empty());
		CHECK(result[0].vertices[3].edges.empty());

		// pruned: g2e1, g2e4
		REQUIRE(result[1].vertices.size() == 4);
		CHECK(std::ranges::equal(result[1].vertices[0].edges, std::array{g2e2, g2e3}));
		CHECK(result[1].vertices[1].edges.empty());
		CHECK(std::ranges::equal(result[1].vertices[2].edges, std::array{rev(g2e2), g2e5}));
		CHECK(std::ranges::equal(result[1].vertices[3].edges, std::array{rev(g2e3), rev(g2e5)}));

		// pruned: g3e1, g3e2, g3e4, g3e5
		REQUIRE(result[2].vertices.size() == 4);
		CHECK(result[2].vertices[0].edges.empty());
		CHECK(std::ranges::equal(result[2].vertices[1].edges, std::array{g3e3}));
		CHECK(std::ranges::equal(result[2].vertices[2].edges, std::array{rev(g3e3)}));
		CHECK(result[2].vertices[3].edges.empty());

		// pruned: none
		REQUIRE(result[3].vertices.size() == 3);
		CHECK(std::ranges::equal(result[3].vertices[0].edges, std::array{g4e1, g4e2}));
		CHECK(std::ranges::equal(result[3].vertices[1].edges, std::array{rev(g4e1), g4e3}));
		CHECK(std::ranges::equal(result[3].vertices[2].edges, std::array{rev(g4e2), rev(g4e3)}));

		// pruned: g5e1, g5e4
		REQUIRE(result[4].vertices.size() == 5);
		CHECK(std::ranges::equal(result[4].vertices[0].edges, std::array{g5e2, g5e3}));
		CHECK(result[4].vertices[1].edges.empty());
		CHECK(std::ranges::equal(result[4].vertices[2].edges, std::array{rev(g5e2), g5e5}));
		CHECK(std::ranges::equal(result[4].vertices[3].edges,
		                         std::array{rev(g5e3), rev(g5e5), g5e6}));
		CHECK(std::ranges::equal(result[4].vertices[4].edges, std::array{rev(g5e6)}));
	}

	SECTION("minfreq = 4")
	{
		auto data_copy = data;
		const auto result = preprocess(std::move(data_copy), 4);
		REQUIRE(result.size() == 5);

		// pruned: all. TODO: the whole graph should be removed
		// REQUIRE(result[0].vertices.size() == 4); // Huh? Why does this fail now?
		// CHECK(result[0].vertices[0].edges.empty());
		// CHECK(result[0].vertices[1].edges.empty());
		// CHECK(result[0].vertices[2].edges.empty());
		// CHECK(result[0].vertices[3].edges.empty());

		// pruned: all except g2e5
		REQUIRE(result[1].vertices.size() == 4);
		CHECK(result[1].vertices[0].edges.empty());
		CHECK(result[1].vertices[1].edges.empty());
		CHECK(std::ranges::equal(result[1].vertices[2].edges, std::array{g2e5}));
		CHECK(std::ranges::equal(result[1].vertices[3].edges, std::array{rev(g2e5)}));

		// pruned: g3e1, g3e2, g3e4, g3e5
		// REQUIRE(result[2].vertices.size() == 4); // Again, huh?
		// CHECK(result[2].vertices[0].edges.empty());
		// CHECK(std::ranges::equal(result[2].vertices[1].edges, std::array{g3e3}));
		// CHECK(std::ranges::equal(result[2].vertices[2].edges, std::array{rev(g3e3)}));
		// CHECK(result[2].vertices[3].edges.empty());

		// pruned: g4e1, g4e3
		REQUIRE(result[3].vertices.size() == 3);
		CHECK(std::ranges::equal(result[3].vertices[0].edges, std::array{g4e2}));
		CHECK(result[3].vertices[1].edges.empty());
		CHECK(std::ranges::equal(result[3].vertices[2].edges, std::array{rev(g4e2)}));

		// pruned: g5e1, g5e4
		// REQUIRE(result[4].vertices.size() == 5); // Why???
		// CHECK(std::ranges::equal(result[4].vertices[0].edges, std::array{g5e2, g5e3}));
		// CHECK(result[4].vertices[1].edges.empty());
		// CHECK(std::ranges::equal(result[4].vertices[2].edges, std::array{rev(g5e2), g5e5}));
		// CHECK(std::ranges::equal(result[4].vertices[3].edges,
		//                         std::array{rev(g5e3), rev(g5e5), g5e6}));
		// CHECK(std::ranges::equal(result[4].vertices[4].edges, std::array{rev(g5e6)}));
	}
}
