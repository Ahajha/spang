#pragma once

#include <cstdint>
// For std::hash
#include <memory>

namespace spang
{

/*!
Performs a lexicographic comparison on two tuples, produced by 'unzipping' the parameters.
Returns true iff the first tuple compares less than or equal to the second. For example,
parameters (1,3,5,2,4,6) would produce the comparison (1,5,4) <= (3,2,6).
*/
template <class T, class... Ts>
[[nodiscard]] constexpr bool lexicographic_leq(const T& t1, const T& t2, const Ts&... rest)
{
	if constexpr (sizeof...(Ts) == 0)
		return t1 <= t2;
	else
		return (t1 != t2) ? (t1 < t2) : lexicographic_leq(rest...);
}

/*!
Performs a lexicographic comparison on two tuples, produced by 'unzipping' the parameters.
Returns true iff the first tuple compares less than the second. For example, parameters
(3,3,4,6,7,8) would produce the comparison (3,4,7) < (3,6,8).
*/
template <class T, class... Ts>
[[nodiscard]] constexpr bool lexicographic_less(const T& t1, const T& t2, const Ts&... rest)
{
	if constexpr (sizeof...(Ts) == 0)
		return t1 < t2;
	else
		return (t1 != t2) ? (t1 < t2) : lexicographic_less(rest...);
}

// Based on boost::hash_combine
template <typename T> void hash_combine(std::size_t& seed, T value)
{
	seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

} // namespace spang
