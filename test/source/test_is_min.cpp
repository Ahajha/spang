#include <spang/is_min.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>

using spang::dfs_edge_t;
using spang::is_min;

TEST_CASE("is_min")
{
	CHECK(is_min(std::array{dfs_edge_t{0, 1, 2, 3, 4}})); //
}