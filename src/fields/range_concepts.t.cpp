#include "ContainerTuple.hpp"
#include "types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <range/v3/all.hpp>

#include <cstdlib>
#include <vector>

#include <iostream>

namespace ccs
{
template <typename T>
concept any_output_range = rs::range<T>&& rs::output_range<T, rs::range_value_t<T>>;
}

TEST_CASE("Output Ranges")
{
    using namespace ccs;
    using namespace ccs::field::tuple;

    REQUIRE(rs::output_range<std::vector<real>&, real>);
    REQUIRE(rs::output_range<std::vector<real>&, int>);
    REQUIRE(traits::AnyOutputRange<std::vector<real>&>);
    REQUIRE(traits::OutputRange<std::vector<real>&, real>);

    REQUIRE(!rs::output_range<const std::vector<real>&, real>);
    REQUIRE(!traits::AnyOutputRange<const std::vector<real>&>);

    REQUIRE(traits::AnyOutputRange<std::span<real>>);
    REQUIRE(
        rs::output_range<std::span<real>, rs::range_value_t<decltype(vs::iota(0, 10))>>);
    REQUIRE(!traits::AnyOutputRange<std::span<const real>>);
    REQUIRE(traits::OutputRange<std::span<real>, std::span<const real>>);
}

TEST_CASE("OutputTuple")
{
    using namespace ccs;
    using namespace ccs::field::tuple;

    using T = std::vector<real>;

    REQUIRE(traits::OutputTuple<std::tuple<T, T>, real>);
    REQUIRE(traits::OutputTuple<std::tuple<std::tuple<T>, std::tuple<T, T, T>>, real>);
    REQUIRE(traits::OutputTuple<std::tuple<T, T>, int>);
    REQUIRE(traits::OutputTuple<std::tuple<T&, T&>, T>);
    REQUIRE(!traits::OutputTuple<std::tuple<const T&, const T&>, T>);

    REQUIRE(traits::OutputTuple<std::tuple<std::span<real>>, T>);
    REQUIRE(
        !traits::OutputTuple<std::tuple<std::span<const real>>, std::span<const real>>);

    REQUIRE(traits::OutputTuple<std::tuple<std::span<real>, std::span<real>>,
                                std::tuple<const T&, const T&>>);
    REQUIRE(!traits::OutputTuple<std::tuple<const T&, const T&>,
                                 std::tuple<std::span<real>, std::span<real>>>);
}

TEST_CASE("Modify Containers from Views")
{
    using namespace ccs;

    auto x = std::vector{1, 2, 3};
    auto y = vs::all(x);

    REQUIRE(any_output_range<decltype(y)>);

    for (auto&& i : y) i *= 2;

    REQUIRE(rs::equal(x, std::vector{2, 4, 6}));

    {
        constexpr auto f = [](auto&& i) { return i; };
        auto a = x | vs::transform(f);
        auto b = y | vs::transform(f);
        static_assert(std::same_as<decltype(a), decltype(b)>);
    }
}

TEST_CASE("TupleLike")
{
    using namespace ccs;
    using namespace field::tuple;
    REQUIRE(traits::TupleLike<std::tuple<int, int>>);
    REQUIRE(traits::TupleLike<ContainerTuple<std::vector<int>>>);

    // take_exactly results in a custom range tuple.  Ensure we do not treat it as one of
    // ours.
    auto x = std::vector<int>(50);
    REQUIRE(!traits::TupleLike<decltype(x | vs::take_exactly(5))>);
}

TEST_CASE("NotTupleRanges")
{
    using namespace ccs;
    using namespace ccs::field::tuple;

    REQUIRE(traits::NonTupleRange<std::vector<real>>);
    REQUIRE(traits::NonTupleRange<std::vector<real>&>);
    REQUIRE(traits::NonTupleRange<const std::vector<real>&>);
    REQUIRE(traits::NonTupleRange<std::span<real>>);
    REQUIRE(traits::NonTupleRange<std::span<const real>>);
}

TEST_CASE("From")
{
    using namespace ccs;
    using namespace ccs::field::tuple;

    using T = std::vector<int>;
    using I = decltype(vs::iota(0, 10));
    using Z = decltype(vs::zip_with(std::plus{}, vs::iota(0, 10), vs::iota(1, 11)));

    REQUIRE(traits::is_constructible_from_range<std::span<const int>, T>::value);
    REQUIRE(traits::is_constructible_from_range<T, I>::value);
    REQUIRE(traits::is_constructible_from<std::span<const int>, const T&>::value);
    REQUIRE(traits::is_constructible_from<T, I>::value);
    REQUIRE(traits::is_constructible_from<T, Z>::value);

    REQUIRE(traits::TupleFromTuple<std::tuple<T>, std::tuple<I>>);
    REQUIRE(traits::TupleFromTuple<std::tuple<T, T>, std::tuple<Z, I>>);
    REQUIRE(traits::TupleFromTuple<std::tuple<std::tuple<T>, std::tuple<T, T>>,
                                   std::tuple<std::tuple<Z>, std::tuple<I, Z>>>);
}

TEST_CASE("tuple shape")
{
    using namespace ccs::field::tuple::traits;

    REQUIRE(SimilarTuples<std::tuple<int>, std::tuple<double>>);
    REQUIRE(SimilarTuples<std::tuple<std::tuple<int>>, std::tuple<std::tuple<void*>>>);
    REQUIRE(!SimilarTuples<std::tuple<std::tuple<int>>, std::tuple<void*>>);
    REQUIRE(
        SimilarTuples<std::tuple<std::tuple<int>, std::tuple<int, int, int>>,
                      std::tuple<std::tuple<void*>, std::tuple<void*, char, double>>>);
    REQUIRE(
        !SimilarTuples<std::tuple<std::tuple<int, int, int>, std::tuple<void*>>,
                       std::tuple<std::tuple<void*>, std::tuple<void*, char, double>>>);
}

TEST_CASE("levels")
{
    using namespace ccs::field::tuple::traits;
    REQUIRE(tuple_levels_v<std::tuple<int>> == 1);
    REQUIRE(tuple_levels_v<std::tuple<int, double, float, char, void*>> == 1);
    REQUIRE(tuple_levels_v<std::tuple<std::tuple<int>>> == 2);
    REQUIRE(tuple_levels_v<std::tuple<std::tuple<int>,
                                      std::tuple<float, double, char>,
                                      std::tuple<void*>>> == 2);
}

TEST_CASE("view closures")
{
    using namespace ccs;
    using namespace ccs::field::tuple::traits;

    using I = decltype(vs::transform([](auto&& i) { return i; }));
    REQUIRE(ViewClosure<I>);
    REQUIRE(ViewClosures<I>);
    REQUIRE(ViewClosures<std::tuple<I, I>>);
}

TEST_CASE("list index")
{
    using namespace ccs::field::tuple::traits;

    using L = list_index<4, 5, 6>;
    REQUIRE(ListIndex<L>);
    static_assert(index_v<L, 0> == 4);
    static_assert(index_v<L, 1> == 5);
    static_assert(index_v<L, 2> == 6);

    using ListOfL = mp_list<list_index<0, 1, 2>, list_index<1>>;
    REQUIRE(ListIndices<ListOfL>);
}

TEST_CASE("viewable ranges")
{
    using namespace ccs::field::tuple::traits;
    static_assert(std::same_as<viewable_range_by_value<std::span<int>>, std::span<int>>);
    static_assert(std::same_as<viewable_range_by_value<std::span<int>&>, std::span<int>>);
    static_assert(std::same_as<viewable_range_by_value<std::span<const int>>,
                               std::span<const int>>);
    static_assert(std::same_as<viewable_range_by_value<std::span<const int>&>,
                               std::span<const int>>);
    static_assert(
        std::same_as<viewable_range_by_value<std::vector<int>&>, std::vector<int>&>);
    static_assert(std::same_as<viewable_range_by_value<const std::vector<int>&>,
                               const std::vector<int>&>);
}

TEST_CASE("NumericTupleLike")
{
    using namespace ccs;
    using namespace field::tuple::traits;

    REQUIRE(NumericTupleLike<real3>);
    REQUIRE(NumericTupleLike<std::tuple<real, int>>);
    REQUIRE(NumericTupleLike<std::tuple<real&, const int&>>);
    REQUIRE(!NumericTupleLike<std::tuple<std::vector<int>>>);
    using T = rs::common_tuple<const int&, const int&, const int&>;
    REQUIRE(TupleLike<T>);
    REQUIRE(NumericTupleLike<rs::common_tuple<const int&, const int&, const int&>>);
}
