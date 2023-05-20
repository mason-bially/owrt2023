#pragma once

#include <tuple>
#include <stdexcept>
#include <cmath>
#include <iostream>

namespace common
{
    /* Concepts */

    template<typename T>
    concept N2 = requires {
            typename T::Num;
            std::tuple_size_v<T> == 2;
        } and requires(T n) {
            n.size() == 2;
            { get<0>(n) } -> std::convertible_to<typename T::Num>;
            { get<1>(n) } -> std::convertible_to<typename T::Num>;
        };

    template<typename T>
    concept N3 = requires {
            typename T::Num;
            std::tuple_size_v<T> == 3;
        } and requires(T n) {
            n.size() == 3;
            { get<0>(n) } -> std::convertible_to<typename T::Num>;
            { get<1>(n) } -> std::convertible_to<typename T::Num>;
            { get<2>(n) } -> std::convertible_to<typename T::Num>;
        };

    template<typename T>
    concept N4 = requires {
            typename T::Num;
            std::tuple_size_v<T> == 4;
        } and requires(T n) {
            n.size() == 4;
            { get<0>(n) } -> std::convertible_to<typename T::Num>;
            { get<1>(n) } -> std::convertible_to<typename T::Num>;
            { get<2>(n) } -> std::convertible_to<typename T::Num>;
            { get<3>(n) } -> std::convertible_to<typename T::Num>;
        };


    /* Utility Functions */

    // TODO ostream concept?
    constexpr auto operator<<(std::ostream &out, N3 auto const& n) -> std::ostream&
    {
        return out << get<0>(n) << ' ' << get<1>(n) << ' ' << get<2>(n);
    }
    constexpr auto operator<<(std::ostream &out, N4 auto const& n) -> std::ostream&
    {
        return out << get<0>(n) << ' ' << get<1>(n) << ' ' << get<2>(n) << ' ' << get<3>(n);
    }
}
