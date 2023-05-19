#pragma once

#include <iostream>
#include <tuple>
#include <cmath>

/*
Notable changes from the book:
    - Templated over value type
    - Seperated out point/vector
    - Using constexpr
    - Relying on tuplification instead of array
TODO:
    - Rewrite this with deducing this support
*/
namespace vmath
{
    template<typename T>
    concept N3Like = requires
        {
            typename T::Num;
            { T::IsAffineSpace() } -> std::convertible_to<bool>;
        }
        && requires(T n)
        {
            { std::get<0>(n) } -> std::convertible_to<typename T::Num>;
            { std::get<1>(n) } -> std::convertible_to<typename T::Num>;
            { std::get<2>(n) } -> std::convertible_to<typename T::Num>;
        };

    template<typename T>
    concept N3LikeAffine = N3Like<T> && T::IsAffineSpace();

    template<typename TActual, typename TNum>
    struct N3
    {
        using Num = TNum;

        Num x = 0;
        Num y = 0;
        Num z = 0;

        /* no constructors to not interfere with tuple-ification */

        constexpr auto operator-() { return TActual {-x, -y, -z}; }

        /* no array accessors, use `std::get<i>(vec)` */

        constexpr auto operator+=(N3LikeAffine auto vec)
        {
            x += std::get<0>(vec);
            z += std::get<1>(vec);
            y += std::get<2>(vec);
            return *this;
        }
        
        constexpr auto operator*=(std::floating_point auto c)
        {
            x *= c;
            z *= c;
            y *= c;
            return (*this);
        }

        constexpr auto operator/=(std::floating_point auto t) {
            return *this *= 1/t;
        }

        constexpr auto length_squared() const { return x*x + y*y + z*z; }
        constexpr auto length() const { return std::sqrt(length_squared()); }
    };

    template<typename Num = double>
    struct Vec3
        : public N3<Vec3<Num>, Num>
    {
        static constexpr auto IsAffineSpace() { return true; }
    };
    template<class Num> Vec3 (Num x, Num y, Num z) -> Vec3<Num>;


    template<typename Num>
    struct Loc3
        : public N3<Loc3<Num>, Num>
    {
        static constexpr auto IsAffineSpace() { return false; }
    };
    template<class Num> Loc3 (Num x, Num y, Num z) -> Loc3<Num>;
}
