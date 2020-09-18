#pragma once

#include "types.hpp"

// Orgainization of slow/fast indices for things written as (i, j, k) tuples
// Generally, operators meant to be applied in a particular direction are applied
// to data for which that direction is contiguous and the other two are have non-unit
// strides.  The direction with the biggest strides is `slow` while the direction
// with the smaller strides is `fast`
namespace ccs::index
{

struct index_pairs {
    int fast;
    int slow;
};

template <int I>
struct dir;

template <>
struct dir<0> {
    static constexpr int slow = 1;
    static constexpr int fast = 2;
};

template <>
struct dir<1> {
    static constexpr int slow = 0;
    static constexpr int fast = 2;
};

template <>
struct dir<2> {
    static constexpr int slow = 0;
    static constexpr int fast = 1;
};

constexpr index_pairs dirs(int i)
{
    switch (i) {
    case 0:
        return {dir<0>::fast, dir<0>::slow};
    case 1:
        return {dir<1>::fast, dir<1>::slow};
    default:
        return {dir<2>::fast, dir<2>::slow};
    }
}

// given a coordinate in "A" dir format, return the coordinate in "B" dir format
namespace detail
{
template <int A, int B, int C>
constexpr int pos(int x)
{
    if (x == A) return 0;
    if (x == B) return 1;
    return 2;
}
} // namespace detail

template <int AD, int BD>
constexpr auto transpose(int3 a)
{
    if constexpr (AD == BD) return a;

    constexpr auto AF = dir<AD>::fast;
    constexpr auto AS = dir<AD>::slow;

    constexpr int bd = detail::pos<AD, AF, AS>(BD);
    constexpr int bf = detail::pos<AD, AF, AS>(dir<BD>::fast);
    constexpr int bs = detail::pos<AD, AF, AS>(dir<BD>::slow);
    
    return int3{a[bd], a[bf], a[bs]};
}
} // namespace ccs::index
