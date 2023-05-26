#pragma once

#include <tuple>
#include <stdexcept>
#include <cmath>
#include <random>
#include <iostream>

namespace common
{
    // Constants

    constexpr double infinity = std::numeric_limits<double>::infinity();
    constexpr double pi = 3.1415926535897932385;

    // Utility Functions

    template<typename TNum>
    constexpr auto degrees_to_radians(TNum degrees) {
        return TNum(degrees * pi / 180.0);
    }

    struct RandomState {
        std::mt19937 generator;
    };

    template<typename TNum>
    constexpr auto rand(RandomState& rs, TNum min=TNum(0), TNum max=TNum(1))
    {
        std::uniform_real_distribution<TNum> distribution(min, max);
        return distribution(rs.generator);
    }

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

    template<TupLike TTup>
        requires SimpleTupOps<TTup>
    constexpr auto mix(TTup const& a, TTup const& b)
    {
        return [=](typename TTup::Num v) { return (1-v)*a + v*b; };
    }

    template<TupLike TTup>
        requires SimpleTupOps<TTup>
    constexpr auto mix(TTup const& a, TTup const& b, typename TTup::Num v)
    {
        return mix(a, b)(v);
    }

    template<typename TNum>
    constexpr auto clamp(TNum x, TNum min=TNum(0), TNum max=TNum(1))
    {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    template<TupLike TTup, typename TNum = TTup::Num>
    constexpr auto map(TTup const& tup, std::invocable<TNum> auto f)
    {
        TTup res;
        if constexpr (TTup::size() > 0) get<0>(res) = f(get<0>(tup));
        if constexpr (TTup::size() > 1) get<1>(res) = f(get<1>(tup));
        if constexpr (TTup::size() > 2) get<2>(res) = f(get<2>(tup));
        if constexpr (TTup::size() > 3) get<3>(res) = f(get<3>(tup));
        return res;
    }

    template<TupLike TTup, typename TNum = TTup::Num>
        requires SimpleTupOps<TTup>
    constexpr auto clamp(TTup const& tup, TNum min=TNum(0), TNum max=TNum(1))
    {
        return map(tup, [=](auto v){ return clamp(v, min, max); });
    }

    template<TupLike TTup, typename TNum = TTup::Num>
        requires SimpleTupOps<TTup>
    constexpr auto rand(RandomState& rs, TNum min=TNum(0), TNum max=TNum(1))
    {
        return map(TTup {}, [=,&rs](auto _){ return rand(rs, min, max); });
    }
}
