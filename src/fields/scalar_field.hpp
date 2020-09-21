#pragma once

#include "indexing.hpp"
#include "types.hpp"

#include "result_field.hpp"

#include <range/v3/algorithm/copy.hpp>
#include <range/v3/view/repeat_n.hpp>

namespace ccs
{

namespace vs = ranges::views;

template <typename T = real, int = 2>
class scalar_field;

template <ranges::random_access_range R, int I>
struct scalar_range;

namespace detail
{
// define some traits and concepts to constrain our universal references and ranges
template <typename = void>
struct is_scalar_field : std::false_type {
};
template <typename T, int I>
struct is_scalar_field<scalar_field<T, I>> : std::true_type {
};

template <typename U>
constexpr bool is_scalar_field_v = is_scalar_field<std::remove_cvref_t<U>>::value;

template <typename = void>
struct is_scalar_range : std::false_type {
};
template <typename T, int I>
struct is_scalar_range<scalar_range<T, I>> : std::true_type {
};

template <typename U>
constexpr bool is_scalar_range_v = is_scalar_range<std::remove_cvref_t<U>>::value;

template <typename U>
constexpr bool is_range_or_field_v = is_scalar_field_v<U> || is_scalar_range_v<U>;

template <typename>
struct scalar_dim_;

template <typename T, int I>
struct scalar_dim_<scalar_range<T, I>> {
    static constexpr int dim = I;
};

template <typename T, int I>
struct scalar_dim_<scalar_field<T, I>> {
    static constexpr int dim = I;
};

template <typename T>
requires is_range_or_field_v<std::remove_cvref_t<T>> constexpr int scalar_dim =
    scalar_dim_<std::remove_cvref_t<T>>::dim;

template <typename T, typename U>
requires is_range_or_field_v<T>&& is_range_or_field_v<U> constexpr bool is_same_dim_v =
    scalar_dim<T> == scalar_dim<U>;

} // namespace detail

template <typename T>
concept Field = detail::is_range_or_field_v<T>;

template <typename T, typename U>
concept Compatible_Fields = Field<T>&& Field<U>&& detail::is_same_dim_v<T, U>;

template <typename T, typename U>
concept Transposable_Fields = Field<T>&& Field<U> && (!detail::is_same_dim_v<T, U>);

template <typename R>
concept Index_Range = ranges::input_range<R> &&
                      (!std::same_as<int3, std::remove_cvref_t<R>>);

template <ranges::random_access_range R, int I>
struct scalar_range : result_range<R> {
    int3 extents_;

    int3 extents() { return extents_; }
};

namespace detail
{
template <Index_Range R, typename T, int I>
class scalar_field_select
{
    R indices;
    scalar_field<T, I>& f;

public:
    scalar_field_select(R indices,
                        scalar_field<T, I>& f) requires std::is_lvalue_reference_v<R>
        : indices{indices}, f{f}
    {
    }

    scalar_field_select(R indices, scalar_field<T, I>& f)
        : indices{std::move(indices)}, f{f}
    {
    }

    template <ranges::input_range V>
    void operator=(V&& values)
    {
        for (auto&& [i, v] : vs::zip(indices, values)) f(i) = v;
    }
};
} // namespace detail

template <typename T, int I>
class scalar_field : public result_field<std::vector, T>
{
    using S = scalar_field<T, I>;
    using P = result_field<std::vector, T>;

    using P::f;
    int3 extents_;

public:
    scalar_field() = default;

    scalar_field(int n, int3 ex) : P{n}, extents_{ex} {}

    scalar_field(std::vector<T> f, int3 ex) : P{std::move(f)}, extents_{ex} {}

    // this will not override default copy/move constructors
    template <typename R>
        requires Compatible_Fields<S, R> &&
        (!std::same_as<S, std::remove_cvref<R>>)scalar_field(R&& r)
        : P(r.size()),
    extents_{r.extents()}
    {
        ranges::copy(r, f.begin());
    }

    template <typename R>
    requires Transposable_Fields<S, R> scalar_field(R&& r)
        : P(r.size()), extents_{r.extents()}
    {
        // invoke copy assignment
        *this = r;
    }

    // this will not override default copy/move assigmnent
    template <typename R>
        requires Compatible_Fields<S, R> &&
        (!std::same_as<S, std::remove_cvref<R>>)scalar_field& operator=(R&& r)
    {
        extents_ = r.extents();
        f.resize(r.size());
        ranges::copy(r, f.begin());
        return *this;
    }

#define gen_operators(op, acc)                                                           \
    template <typename R>                                                                \
    requires Transposable_Fields<S, R> scalar_field& op(R&& r)                           \
    {                                                                                    \
        extents_ = r.extents();                                                          \
        f.resize(r.size());                                                              \
                                                                                         \
        constexpr int AD = I;                                                            \
        constexpr int AS = index::dir<AD>::slow;                                         \
        constexpr int AF = index::dir<AD>::fast;                                         \
                                                                                         \
        constexpr int BD = detail::scalar_dim<R>;                                        \
        constexpr int BS = index::dir<BD>::slow;                                         \
        constexpr int BF = index::dir<BD>::fast;                                         \
                                                                                         \
        auto [nad, naf, nas] = int3{extents_[AD], extents_[AF], extents_[AS]};           \
        auto [nbd, nbf, nbs] = int3{extents_[BD], extents_[BF], extents_[BS]};           \
                                                                                         \
        for (int as = 0; as < nas; as++)                                                 \
            for (int af = 0; af < naf; af++)                                             \
                for (int ad = 0; ad < nad; ad++) {                                       \
                    auto [bd, bf, bs] = index::transpose<AD, BD>(int3{ad, af, as});      \
                    f[as * naf * nad + af * nad + ad] acc                                \
                        r[bs * nbf * nbd + bf * nbd + bd];                               \
                }                                                                        \
        return *this;                                                                    \
    }                                                                                    \
    template <Numeric N>                                                                 \
    scalar_field& op(N n)                                                                \
    {                                                                                    \
        for (auto&& v : f) v acc n;                                                \
        return *this;                                                                    \
    }

    // clang-format off
gen_operators(operator=, =)
gen_operators(operator+=, +=)
gen_operators(operator-=, -=)
gen_operators(operator*=, *=)
gen_operators(operator/=, /=)
#undef gen_operators

    // clang-format on

                constexpr int index(const int3& ijk) const
    {
        constexpr int D = I;
        constexpr int S = index::dir<D>::slow;
        constexpr int F = index::dir<D>::fast;
        return ijk[S] * extents_[D] * extents_[F] + ijk[F] * extents_[D] + ijk[D];
    }

    // bring base class call operator into scope so we don't shadow them
    using P::operator();

    const T& operator()(const int3& ijk) const { return f[index(ijk)]; }

    T& operator()(const int3& ijk) { return f[index(ijk)]; }

    template <ranges::random_access_range R>
    requires(!std::same_as<int3, std::remove_cvref_t<R>>) auto operator()(R&& r) &
    {
        return detail::scalar_field_select<R, T, I>{std::forward<R>(r), *this};
    }

    int3 extents() { return extents_; }

    operator std::span<const T>() const { return f; }

    operator result_view_t<T>() { return {f}; }
}; // namespace ccs

#define ret_range(expr)                                                                  \
    return scalar_range<decltype(expr), I> { expr, t.extents() }

#define gen_operators(op, f)                                                             \
    template <typename T, typename U>                                                    \
    requires Compatible_Fields<T, U> constexpr auto op(T&& t, U&& u)                     \
    {                                                                                    \
        assert(t.extents() == u.extents());                                              \
        constexpr auto I = detail::scalar_dim<T>;                                        \
        ret_range(                                                                       \
            vs::zip_with(f, std::forward<T>(t).range(), std::forward<U>(u).range()));    \
    }                                                                                    \
    template <Field T, Numeric U>                                                        \
    constexpr auto op(T&& t, U u)                                                        \
    {                                                                                    \
        constexpr auto I = detail::scalar_dim<T>;                                        \
        ret_range(                                                                       \
            vs::zip_with(f, std::forward<T>(t).range(), vs::repeat_n(u, t.size())));     \
    }                                                                                    \
    template <Field T, Numeric U>                                                        \
    constexpr auto op(U u, T&& t)                                                        \
    {                                                                                    \
        constexpr auto I = detail::scalar_dim<T>;                                        \
        ret_range(                                                                       \
            vs::zip_with(f, vs::repeat_n(u, t.size()), std::forward<T>(t).range()));     \
    }

// clang-format off
gen_operators(operator+, std::plus{})

gen_operators(operator-, std::minus{})

gen_operators(operator*, std::multiplies{})

gen_operators(operator/, std::divides{})
// clang-format on

#undef gen_operators
#undef ret_range

using x_field = scalar_field<real, 0>;
using y_field = scalar_field<real, 1>;
using z_field = scalar_field<real, 2>;

} // namespace ccs