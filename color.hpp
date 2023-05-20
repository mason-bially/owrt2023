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
namespace color
{
    /* Concepts */

    template<typename T>
    concept N3Like = common::N3<T>;

    template<typename T>
    concept N3Full = N3Like<T>;

    /* Classes */

    template<typename TActual, typename TNum>
    struct N3
    {
        using Num = TNum;

        Num r = 0;
        Num g = 0;
        Num b = 0;

        /* no constructors to not interfere with tuple-ification */

        /* prefer `std::get<i>(vec)` for compile time access */

        static constexpr auto size() { return 3; }
        // for runtime array access
        constexpr auto operator[](int i)
        {
            switch(i) {
                case 0: return r;
                case 1: return g; 
                case 2: return b;
                default: throw std::out_of_range("i");
            }
        }

        constexpr auto operator+=(N3Like auto vec)
        {
            r += get<0>(vec);
            g += get<1>(vec);
            b += get<2>(vec);
            return *this;
        }
        
        constexpr auto operator*=(TNum c)
        {
            r *= c;
            g *= c;
            b *= c;
            return (*this);
        }

        constexpr auto operator/=(TNum t) {
            return *this *= 1/t;
        }
    };
    template <unsigned I, typename TActual, typename TNum>
    constexpr auto get (N3<TActual, TNum>& n)
    {
        if constexpr (I == 0) return n.r;
        else if constexpr (I == 1) return n.g;
        else if constexpr (I == 2) return n.b;
        else static_assert("Get Index out of Range");
    }
    template <unsigned I, typename TActual, typename TNum>
    constexpr auto get (N3<TActual, TNum> const& n)
    {
        if constexpr (I == 0) return n.r;
        else if constexpr (I == 1) return n.g;
        else if constexpr (I == 2) return n.b;
        else static_assert("Get Index out of Range");
    }

    template<class TNum = double>
    struct Color3
        : public N3<Color3<TNum>, TNum>
    {
        using Base = N3<Color3<TNum>, TNum>;
        using Base::r;
        using Base::g;
        using Base::b;
    };
    template<class Num> Color3 (Num x, Num y, Num z) -> Color3<Num>;

    /* Utility Functions */

    // TODO ostream concept?
    constexpr auto operator <<(std::ostream &out, N3Full auto c) -> std::ostream&
    {
        // Write the translated [0,255] value of each color component.
        constexpr double color_conv = 255.999;
        return out 
            << static_cast<int>(color_conv * c.r) << ' '
            << static_cast<int>(color_conv * c.g) << ' '
            << static_cast<int>(color_conv * c.b);
    }

    template<N3Full TN3>
    constexpr auto operator+(TN3 const& u, TN3 const& v)
    {
        return TN3 { u.x + v.x, u.y + v.y, u.z + v.z };
    }

    template<N3Full TN3>
    constexpr auto operator-(TN3 const& u, TN3 const& v)
    {
        return TN3 { u.x - v.x, u.y - v.y, u.z - v.z };
    }
    
    template<N3Full TN3>
    constexpr auto operator*(TN3 const& u, TN3 const& v)
    {
        return TN3 { u.x * v.x, u.y * v.y, u.z * v.z };
    }

    template<N3Full TN3>
    constexpr auto operator*(typename TN3::Num t, TN3 const& v)
    {
        return TN3 { t*v.x, t*v.y, t*v.z };
    }

    template<N3Full TN3>
    constexpr auto operator*(TN3 const& v, typename TN3::Num t)
    {
        return t * v;
    }

    template<N3Full TN3>
    constexpr auto operator/(TN3 const& v, typename TN3::Num t)
    {
        return (1/t) * v;
    }
}

    
template <typename TNum>
struct std::tuple_size<color::Color3<TNum>>
    : public integral_constant<std::size_t, color::Color3<TNum>::size()> {};
