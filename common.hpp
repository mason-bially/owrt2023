#pragma once

#include <tuple>
#include <stdexcept>
#include <cmath>
#include <iostream>

namespace common
{
    /* Concepts */

    template<typename T>
    concept TupLikeBase = requires {
        typename T::Num;
    } and requires(T n, size_t i) {
        { n[i] } -> std::convertible_to<typename T::Num>;
    };

    template<typename T>
    concept Tup2Like = TupLikeBase<T>
        and requires {
            std::tuple_size_v<T> == 2;
            std::same_as<std::tuple_element_t<0, T>, typename T::Num>;
            std::same_as<std::tuple_element_t<1, T>, typename T::Num>;
        } and requires(T n) {
            n.size() == 2;
            { get<0>(n) } -> std::convertible_to<typename T::Num>;
            { get<1>(n) } -> std::convertible_to<typename T::Num>;
        };

    template<typename T>
    concept Tup3Like = TupLikeBase<T>
        and requires {
            std::tuple_size_v<T> == 3;
            std::same_as<std::tuple_element_t<0, T>, typename T::Num>;
            std::same_as<std::tuple_element_t<1, T>, typename T::Num>;
            std::same_as<std::tuple_element_t<2, T>, typename T::Num>;
        } and requires(T n) {
            n.size() == 3;
            { get<0>(n) } -> std::convertible_to<typename T::Num>;
            { get<1>(n) } -> std::convertible_to<typename T::Num>;
            { get<2>(n) } -> std::convertible_to<typename T::Num>;
        };

    template<typename T>
    concept Tup4Like = TupLikeBase<T>
        and requires {
            std::tuple_size_v<T> == 4;
            std::same_as<std::tuple_element_t<0, T>, typename T::Num>;
            std::same_as<std::tuple_element_t<1, T>, typename T::Num>;
            std::same_as<std::tuple_element_t<2, T>, typename T::Num>;
            std::same_as<std::tuple_element_t<3, T>, typename T::Num>;
        } and requires(T n) {
            n.size() == 4;
            { get<0>(n) } -> std::convertible_to<typename T::Num>;
            { get<1>(n) } -> std::convertible_to<typename T::Num>;
            { get<2>(n) } -> std::convertible_to<typename T::Num>;
            { get<3>(n) } -> std::convertible_to<typename T::Num>;
        };

    template<typename T>
    concept TupLike = Tup2Like<T> or Tup3Like<T> or Tup4Like<T>;

    template<typename T>
    concept SimpleTupOps = requires(T a, T b) {
        { a + b } -> std::convertible_to<T>;
        { a - b } -> std::convertible_to<T>;
    } and requires(T a, typename T::Num n) {
        { a * n } -> std::convertible_to<T>;
        { n * a } -> std::convertible_to<T>;
        { a / n } -> std::convertible_to<T>;
    };


    /* Template Helpers */
    template<class... TNums>
    struct numeric_promote {
        static constexpr bool has_double {(std::is_same_v<double, TNums> || ...)};
        static constexpr bool has_float {(std::is_same_v<float, TNums> || ...)};
        static constexpr bool has_int64 {(std::is_same_v<int64_t, TNums> || ...)};
        static constexpr bool has_uint64 {(std::is_same_v<uint64_t, TNums> || ...)};
        using back_up_type = std::tuple_element_t<0, std::tuple<TNums...>>;
        using uint64_or_back = std::conditional_t<has_uint64, uint64_t, back_up_type>;
        using int64_or_back = std::conditional_t<has_int64, int64_t, uint64_or_back>;
        using float_or_back = std::conditional_t<has_float, float, int64_or_back>;
        using double_or_back = std::conditional_t<has_double, double, float_or_back>;

        using type = double_or_back;
    };
    template<class... TNums>
    using numeric_promote_t = typename numeric_promote<TNums...>::type;

    /* Utility Functions */

    // TODO ostream concept?
    constexpr auto operator<<(std::ostream &out, TupLike auto const& n) -> std::ostream&
    {
        out << get<0>(n) << ' ' << get<1>(n);
        if constexpr (n.size() > 2)
            out << ' ' << get<2>(n);
        if constexpr (n.size() > 3)
            out << ' ' << get<3>(n);
        return out;
    }

    template<TupLike N>
        requires SimpleTupOps<N>
    constexpr auto mix(N const& a, N const& b)
    {
        return [=](typename N::Num v) { return (1-v)*a + v*b; };
    }

    template<TupLike N>
        requires SimpleTupOps<N>
    constexpr auto mix(N const& a, N const& b, typename N::Num v)
    {
        return mix(a, b)(v);
    }
}
