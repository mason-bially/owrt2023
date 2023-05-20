#pragma once

#include <tuple>
#include <stdexcept>
#include <cmath>
#include <iostream>

#include "common.hpp"

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
    /* Concepts */

    template<typename T>
    concept IsAffine = T::IsAffineSpace();

    template<typename T>
    concept N3Like = common::N3<T> and requires {
            { T::IsAffineSpace() } -> std::convertible_to<bool>;
        } and requires(T n) {
            { affine(n) } -> IsAffine;
        };

    template<typename T>
    concept N3Full = N3Like<T> and requires(T n) {
            { affine(n) } -> N3Like;
    };

    template<typename T> concept N3Affine = N3Full<T> and IsAffine<T>;
    template<typename T> concept N3NonAffine = N3Full<T> and !IsAffine<T>;

    /* Classes */

    template<typename TActual, typename TNum>
    struct N3
    {
        using Num = TNum;

        Num x = 0;
        Num y = 0;
        Num z = 0;

        /* no constructors to not interfere with tuple-ification */

        constexpr auto operator-() { return TActual {-x, -y, -z}; }

        /* prefer `std::get<i>(vec)` for compile time access */

        constexpr auto size() { return std::tuple_size_v<TActual>; }
        // for runtime array access
        constexpr auto operator[](int i)
        {
            switch(i) {
                case 0: return x;
                case 1: return y; 
                case 2: return z;
                default: throw std::out_of_range("i");
            }
        }

        constexpr auto operator+=(N3Affine auto vec)
        {
            x += get<0>(vec);
            z += get<1>(vec);
            y += get<2>(vec);
            return *this;
        }
        
        constexpr auto operator*=(TNum c)
        {
            x *= c;
            z *= c;
            y *= c;
            return (*this);
        }

        constexpr auto operator/=(TNum t) {
            return *this *= 1/t;
        }

        constexpr auto length_squared() const { return x*x + y*y + z*z; }
        constexpr auto length() const { return std::sqrt(length_squared()); }
    };
    template <unsigned I, typename TActual, typename TNum>
    constexpr auto get (N3<TActual, TNum>& n)
    {
        if constexpr (I == 0) return n.x;
        else if constexpr (I == 1) return n.x;
        else if constexpr (I == 2) return n.x;
        else static_assert("Get Index out of Range");
    }
    template <unsigned I, typename TActual, typename TNum>
    constexpr auto get (N3<TActual, TNum> const& n)
    {
        if constexpr (I == 0) return n.x;
        else if constexpr (I == 1) return n.x;
        else if constexpr (I == 2) return n.x;
        else static_assert("Get Index out of Range");
    }


    template<class Num = double>
    struct Vec3
        : public N3<Vec3<Num>, Num>
    {
        using Base = N3<Vec3<Num>, Num>;
        using Base::x;
        using Base::y;
        using Base::z;

        static constexpr auto IsAffineSpace() { return true; }
    };
    template<class Num> Vec3 (Num x, Num y, Num z) -> Vec3<Num>;
    template<class Num> constexpr auto affine (Vec3<Num> n) { return n; }


    template<class Num = double>
    struct Loc3
        : public N3<Loc3<Num>, Num>
    {
        using Base = N3<Loc3<Num>, Num>;
        using Base::x;
        using Base::y;
        using Base::z;

        static constexpr auto IsAffineSpace() { return false; }

        constexpr explicit operator Vec3<Num>() { return Vec3 { x, y, z }; }
    };
    template<class Num> Loc3 (Num x, Num y, Num z) -> Loc3<Num>;
    template<class Num> constexpr auto affine (Loc3<Num> n) { return (Vec3<Num>) n; }

    /* Utility Functions */

    template<N3Full TN3A, N3Full TN3B>
    constexpr auto operator+(TN3A const& u, TN3B const& v)
    {
        if constexpr (IsAffine<TN3B>)
            return TN3A { u.x + v.x, u.y + v.y, u.z + v.z };
        else if constexpr (IsAffine<TN3A> and !IsAffine<TN3B>)
            return TN3B { u.x + v.x, u.y + v.y, u.z + v.z };
        else
            static_assert(IsAffine<TN3A> or IsAffine<TN3B>,
                "Cannont operate across two non-affine types.");
    }
    template<N3Full TN3A, N3Full TN3B>
    constexpr auto operator-(TN3A const& u, TN3B const& v)
    {
        if constexpr (IsAffine<TN3B>)
            return TN3A { u.x - v.x, u.y - v.y, u.z - v.z };
        else if constexpr (IsAffine<TN3A> and !IsAffine<TN3B>)
            return TN3B { u.x - v.x, u.y - v.y, u.z - v.z };
        else
            static_assert(IsAffine<TN3A> or IsAffine<TN3B>,
                "Cannont operate across two non-affine types.");
    }

    template<N3Affine TN3>
    constexpr auto operator*(typename TN3::Num t, TN3 const& v)
    {
        return TN3 { t*v.x, t*v.y, t*v.z };
    }

    template<N3Affine TN3>
    constexpr auto operator*(TN3 const& v, typename TN3::Num t)
    {
        return t * v;
    }

    template<N3Affine TN3>
    constexpr auto operator/(TN3 const& v, typename TN3::Num t)
    {
        return (1/t) * v;
    }

    template<N3Affine TN3>
    constexpr auto dot(TN3 const& u, TN3 const& v)
    {
        auto [ux, uy, uz] = u; auto [vx, vy, vz] = v;
        return ux * vx + uy * vy + uz * vy;
    }

    template<N3Affine TN3>
    constexpr auto cross(TN3 const& u, TN3 const& v)
    {
        auto [ux, uy, uz] = u; auto [vx, vy, vz] = v;
        return TN3 {
            uy * vz - uz * vy,
            uz * vx - ux * vz,
            ux * vy - uy * vx
        };
    }

    constexpr auto unit_vector(N3Full auto const& v) { return affine(v) / v.length(); }
}
