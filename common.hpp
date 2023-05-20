#pragma once

#include <tuple>
#include <stdexcept>
#include <cmath>
#include <iostream>

namespace common
{
    /* Concepts */

    template<typename T>
    concept CN2Like = requires {
            typename T::Num;
            std::tuple_size_v<T> == 2;
        } and requires(T n) {
            n.size() == 2;
            { get<0>(n) } -> std::convertible_to<typename T::Num>;
            { get<1>(n) } -> std::convertible_to<typename T::Num>;
        };

    template<typename T>
    concept CN3Like = requires {
            typename T::Num;
            std::tuple_size_v<T> == 3;
        } and requires(T n) {
            n.size() == 3;
            { get<0>(n) } -> std::convertible_to<typename T::Num>;
            { get<1>(n) } -> std::convertible_to<typename T::Num>;
            { get<2>(n) } -> std::convertible_to<typename T::Num>;
        };

    template<typename T>
    concept CN4Like = requires {
            typename T::Num;
            std::tuple_size_v<T> == 4;
        } and requires(T n) {
            n.size() == 4;
            { get<0>(n) } -> std::convertible_to<typename T::Num>;
            { get<1>(n) } -> std::convertible_to<typename T::Num>;
            { get<2>(n) } -> std::convertible_to<typename T::Num>;
            { get<3>(n) } -> std::convertible_to<typename T::Num>;
        };

    template<typename T>
    concept CNLike = CN2Like<T> or CN3Like<T> or CN4Like<T>;


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
    constexpr auto operator<<(std::ostream &out, CN2Like auto const& n) -> std::ostream&
    {
        return out << get<0>(n) << ' ' << get<1>(n);
    }
    constexpr auto operator<<(std::ostream &out, CN3Like auto const& n) -> std::ostream&
    {
        return out << get<0>(n) << ' ' << get<1>(n) << ' ' << get<2>(n);
    }
    constexpr auto operator<<(std::ostream &out, CN4Like auto const& n) -> std::ostream&
    {
        return out << get<0>(n) << ' ' << get<1>(n) << ' ' << get<2>(n) << ' ' << get<3>(n);
    }

    template<CNLike N>
    constexpr auto mix(N const& a, N const& b)
    {
        return [=](typename N::Num v) { return (1-v)*a + v*b; };
    }

    template<CNLike N>
    constexpr auto mix(N const& a, N const& b, typename N::Num v)
    {
        return mix(a, b)(v);
    }
}
