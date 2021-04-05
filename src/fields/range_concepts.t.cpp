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
}

namespace ccs
{
// template <int I>
// just do a y-plane view for now
class PlaneView : public rs::view_facade<PlaneView>
{
    friend rs::range_access;
    int nx, ny, nz;
    int j;
    std::span<int> grid;

    struct cursor {
    private:
        std::span<int>::iterator iter;
        int nx, ny, nz;
        int i, j, k;

    public:
        cursor() = default;
        cursor(std::span<int>::iterator iter, int nx, int ny, int nz, int i, int j, int k)
            : iter{iter}, nx{nx}, ny{ny}, nz{nz}, i{i}, j{j}, k{k}
        {
        }
        int& read() const { return *iter; }
        bool equal(const cursor& that) const { return iter == that.iter; }
        void next()
        {
            ++k;
            ++iter;
            if (k == nz) {
                k = 0;
                ++i;
                iter += (ny - 1) * nz;
            }
        }

        void prev()
        {
            --k;
            --iter;
            if (k < 0) {
                k = nz - 1;
                --i;
                iter -= (ny - 1) * nz;
            }
        }

        std::ptrdiff_t distance_to(const cursor& that) const
        {
            return (that.i - i) * nz + (that.k - k);
        }
    };

    cursor begin_cursor() { return {grid.begin() + j * nz, nx, ny, nz, 0, j, 0}; }
    cursor end_cursor()
    {
        return {grid.begin() + (j + 1) * nz, nx, ny, nz, nx - 1, j, nz};
    }

public:
    PlaneView() = default;
    explicit constexpr PlaneView(int3 extents, int j, std::span<int> rng)
        : nx{extents[0]}, ny{extents[1]}, nz{extents[2]}, j{j}, grid{rng}
    {
    }
};

template <typename Rng>
class PlaneViewY : public rs::view_adaptor<PlaneViewY<Rng>, Rng>
{
    using diff_t = rs::range_difference_t<Rng>;

    friend rs::range_access;
    diff_t nx, ny, nz, j;

    class adaptor : public rs::adaptor_base
    {
        diff_t nx, ny, nz, i, j, k;
        // friend class sentinel_adaptor;

    public:
        adaptor() = default;
        adaptor(diff_t nx, diff_t ny, diff_t nz, diff_t i, diff_t j, diff_t k)
            : nx{nx}, ny{ny}, nz{nz}, i{i}, j{j}, k{k}
        {
        }

        template <typename R>
        constexpr auto begin(R& rng)
        {
            auto it = rs::begin(rng.base());
            rs::advance(it, j * nz);
            return it;
        }

        template <typename R>
        constexpr auto end(R& rng)
        {
            auto it = rs::begin(rng.base());
            rs::advance(it, i * ny * nz + j * nz + k);
            return it;
        }

        template <typename I>
        void next(I& it)
        {
            ++k;
            ++it;
            if (k == nz && i != nx - 1) {
                k = 0;
                ++i;
                it += (ny - 1) * nz;
            }
        }

        template <typename I>
        void prev(I& it)
        {
            --k;
            --it;
            if (k < 0) {
                k = nz - 1;
                --i;
                it -= (ny - 1) * nz;
            }
        }

        template <typename I>
        void advance(I& it, rs::difference_type_t<I> n)
        {
            if (n == 0) return;

            const auto line_offset = ny * nz;
            // define a new i and k for the adaptor and adjust iterator accordingly.

            if (n > 0) {
                n += k;

                auto qr = std::div(n, nz);
                diff_t i1 = i + qr.quot;
                diff_t k1 = qr.rem;

                if (i1 == nx) {
                    i1 = nx - 1;
                    k1 = nz;
                }

                rs::advance(it, line_offset * (i1 - i) + (k1 - k));
                i = i1;
                k = k1;
            } else {
                n -= (nz - 1 - k);

                auto qr = std::div(n, nz);
                diff_t i1 = i + qr.quot;
                diff_t k1 = nz - 1 + qr.rem;

                rs::advance(it, line_offset * (i1 - i) + (k1 - k));
                i = i1;
                k = k1;
            }
        }

        template <typename I>
        diff_t distance_to(const I&, const I&, const adaptor& that) const
        {
            return (that.i - i) * nz + (that.k - k);
        }
    };

    adaptor begin_adaptor() { return {nx, ny, nz, 0, j, 0}; }
    adaptor end_adaptor() { return {nx, ny, nz, nx - 1, j, nz}; }

public:
    PlaneViewY() = default;
    explicit constexpr PlaneViewY(Rng&& rng, const int3& extents, int j)
        : PlaneViewY::view_adaptor{FWD(rng)},
          nx{extents[0]},
          ny{extents[1]},
          nz{extents[2]},
          j{j}
    {
    }
};

template <typename Rng>
PlaneViewY(Rng&&, const int3&, int) -> PlaneViewY<Rng>;

struct y_plane_base_fn {
    template <typename Rng>
    constexpr auto operator()(Rng&& rng, const int3& extents, int j) const
    {
        return PlaneViewY(FWD(rng), extents, j);
    }
};

struct y_plane_fn : y_plane_base_fn {
    using y_plane_base_fn::operator();

    constexpr auto operator()(const int3& extents, int j) const
    {
        return rs::make_view_closure(rs::bind_back(y_plane_base_fn{}, extents, j));
    }
};

constexpr auto y_plane_view = y_plane_fn{};

struct plane_base_fn {
    constexpr auto operator()(std::span<int> rng, const int3& extents, int j) const
    {
        return PlaneView(extents, j, rng);
    }
};

struct plane_fn : plane_base_fn {
    using plane_base_fn::operator();

    constexpr auto operator()(const int3& extents, int j) const
    {
        return rs::make_view_closure(rs::bind_back(plane_base_fn{}, extents, j));
    }
};

constexpr auto plane_view = plane_fn{};

struct MyRange : ranges::view_facade<MyRange> {
private:
    friend ranges::range_access;
    std::vector<int> ints_;
    struct cursor {
    private:
        std::vector<int>::iterator iter;

    public:
        using contiguous = std::true_type;
        cursor() = default;
        cursor(std::vector<int>::iterator it) : iter(it) {}
        int& read() const { return *iter; }
        bool equal(cursor const& that) const { return iter == that.iter; }
        void next() { ++iter; }
        void prev() { --iter; }
        std::ptrdiff_t distance_to(cursor const& that) const { return that.iter - iter; }
        void advance(std::ptrdiff_t n) { iter += n; }
    };
    cursor begin_cursor() { return {ints_.begin()}; }
    cursor end_cursor() { return {ints_.end()}; }

public:
    MyRange() : ints_{1, 2, 3, 4, 5, 6, 7} {}
};

} // namespace ccs

TEST_CASE("yplanes")
{
    using namespace ccs;
    using namespace field::tuple;
    using T = std::vector<int>;

    int nx = 3, ny = 5, nz = 4;
    int3 extents{nx, ny, nz};

    auto grid = vs::iota(0, nx * ny * nz);

    {
        auto ymin_plane = grid | vs::chunk(ny * nz) | vs::for_each(vs::take_exactly(nz));
        REQUIRE(rs::equal(ymin_plane | vs::take(nz), vs::iota(0, nz)));
        REQUIRE(rs::equal(ymin_plane | vs::drop(nz) | vs::take(nz),
                          vs::iota(ny * nz, ny * nz + nz)));
    }

    // try another formulation
    {
        auto ymin_plane = grid | vs::chunk(nz) | vs::stride(ny) | vs::join;
        REQUIRE(rs::equal(ymin_plane | vs::take(nz), vs::iota(0, nz)));
        REQUIRE(rs::equal(ymin_plane | vs::drop(nz) | vs::take(nz),
                          vs::iota(ny * nz, ny * nz + nz)));
    }

    {

        static_assert(std::same_as<int, rs::range_value_t<MyRange>>);
        static_assert(rs::output_range<MyRange, rs::range_value_t<MyRange>>);
        static_assert(traits::AnyOutputRange<MyRange>);

        auto rng = MyRange{};
        std::cout << "myRange: " << rng << '\n';

        int sz = rs::size(rng);
        rs::copy(vs::iota(10, 10 + sz), rs::begin(rng));
        std::cout << "myRange: " << rng << '\n';

        static_assert(std::same_as<int, rs::range_value_t<PlaneView>>);
        static_assert(traits::AnyOutputRange<PlaneView>);

        static_assert(rs::range<PlaneView>);
        static_assert(rs::sized_range<PlaneView>);

        auto v = grid | rs::to<T>();
        // auto ymin_plane = PlaneView{extents, 0, v};
        // auto ymin_plane = v | plane_view(extents, 0);
        auto ymin_plane = v | y_plane_view(extents, 0);
        using P = decltype(ymin_plane);
        static_assert(rs::range<P>);
        static_assert(rs::sized_range<P>);
        // ideally would like to do something like v | PlaneView{extents, 0}

        REQUIRE((int)rs::size(ymin_plane) == nx * nz);

        {
            auto beg = rs::begin(ymin_plane);
            REQUIRE(*beg == 0);
            REQUIRE(*++beg == 1);
            REQUIRE(*++beg == 2);
            REQUIRE(*++beg == 3);
            REQUIRE(*++beg == ny * nz);
        }

        {
            auto beg = rs::begin(ymin_plane);
            rs::advance(beg, 0);
            REQUIRE(*beg == 0);
            rs::advance(beg, 1);
            REQUIRE(*beg == 1);
            rs::advance(beg, 1);
            REQUIRE(*beg == 2);
            rs::advance(beg, 1);
            REQUIRE(*beg == 3);
            rs::advance(beg, 1);
            REQUIRE(*beg == ny * nz);
            rs::advance(beg, nz + 1);
            REQUIRE(*beg == 2 * ny * nz + 1);
            rs::advance(beg, -1);
            REQUIRE(*beg == 2 * ny * nz);
            ++beg;
            rs::advance(beg, -(nz + 1));
            REQUIRE(*beg == ny * nz);
        }

        {
            REQUIRE(*(rs::begin(v) + nz) == 4);
            auto first = rs::begin(ymin_plane);
            auto last = rs::end(ymin_plane);
            rs::advance(first, rs::distance(first, last));
            REQUIRE(rs::distance(first, last) == 0);
            REQUIRE(first == last);
        }

        REQUIRE(rs::equal(ymin_plane | vs::take(nz), vs::iota(0, nz)));
        REQUIRE(rs::equal(ymin_plane | vs::drop(nz) | vs::take(nz),
                          vs::iota(ny * nz, ny * nz + nz)));

        rs::copy(vs::iota(0, nx * nz), rs::begin(ymin_plane));
        REQUIRE(rs::equal(v | vs::chunk(nz) | vs::stride(ny) | vs::join,
                          vs::iota(0, nx * nz)));
    }
}