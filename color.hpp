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
    using common::mix;
    using common::clamp;

    /* Concepts */

    template<typename T>
    concept Tup3Like = common::Tup3Like<T>
        and requires(T c) {
            { c.r } -> std::convertible_to<typename T::Num>;
            { c.g } -> std::convertible_to<typename T::Num>;
            { c.b } -> std::convertible_to<typename T::Num>;
        };

    /* Classes */

    template<typename TActual, typename TNum>
    struct Tup3Base
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

        constexpr auto operator+=(Tup3Like auto vec)
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
    constexpr auto& get(Tup3Base<TActual, TNum>& n)
    {
        if constexpr (I == 0) return n.r;
        else if constexpr (I == 1) return n.g;
        else if constexpr (I == 2) return n.b;
        else static_assert("Get Index out of Range");
    }
    template <unsigned I, typename TActual, typename TNum>
    constexpr auto get(Tup3Base<TActual, TNum> const& n)
    {
        if constexpr (I == 0) return n.r;
        else if constexpr (I == 1) return n.g;
        else if constexpr (I == 2) return n.b;
        else static_assert("Get Index out of Range");
    }

    template<class TNum = double>
    struct Color3
        : public Tup3Base<Color3<TNum>, TNum>
    {
        using Num = TNum;

        using Self = Color3<TNum>;
        using Base = Tup3Base<Self, TNum>;

        using Base::r;
        using Base::g;
        using Base::b;

        static constexpr Num NumZero = 0;
        static constexpr Num NumLimit = std::is_floating_point_v<Num> ? 1.0 : std::numeric_limits<TNum>::max();

        static constexpr Self White { NumLimit, NumLimit, NumLimit };
        static constexpr Self Black { NumZero, NumZero, NumZero };
        static constexpr Self Red { NumLimit, NumZero, NumZero };
        static constexpr Self Green { NumZero, NumLimit, NumZero };
        static constexpr Self Blue { NumZero, NumZero, NumLimit };
        static constexpr Self Yellow { NumLimit, NumLimit, NumZero };
        static constexpr Self Teal { NumZero, NumLimit, NumLimit };
        static constexpr Self Magenta { NumLimit, NumZero, NumLimit };
    };
    template<class Num> Color3 (Num x, Num y, Num z) -> Color3<Num>;

    /* Utility Functions */

    // TODO ostream concept?
    constexpr auto operator <<(std::ostream &out, Tup3Like auto c) -> std::ostream&
    {
        // Write the translated [0,255] value of each color component.
        return out 
            << static_cast<int64_t>(c.r) << ' '
            << static_cast<int64_t>(c.g) << ' '
            << static_cast<int64_t>(c.b);
    }

    template<Tup3Like TDst, Tup3Like TSrc>
    constexpr auto color_cast(TSrc src) -> TDst
    {
        return TDst {
            static_cast<typename TDst::Num>(src.r),
            static_cast<typename TDst::Num>(src.g),
            static_cast<typename TDst::Num>(src.b)
        };
    }

    template<Tup3Like TN3>
    constexpr auto operator+(TN3 const& u, TN3 const& v)
    {
        return TN3 { u.r + v.r, u.g + v.g, u.b + v.b };
    }

    template<Tup3Like TN3>
    constexpr auto operator-(TN3 const& u, TN3 const& v)
    {
        return TN3 { u.r - v.r, u.g - v.g, u.b - v.b };
    }
    
    template<Tup3Like TN3>
    constexpr auto operator*(TN3 const& u, TN3 const& v)
    {
        return TN3 { u.r * v.r, u.g * v.g, u.b * v.b };
    }

    template<Tup3Like TN3>
    constexpr auto operator*(typename TN3::Num t, TN3 const& v)
    {
        return TN3 { t*v.r, t*v.g, t*v.b };
    }

    template<Tup3Like TN3>
    constexpr auto operator*(TN3 const& v, typename TN3::Num t)
    {
        return t * v;
    }

    template<Tup3Like TN3>
    constexpr auto operator/(TN3 const& v, typename TN3::Num t)
    {
        return (1/t) * v;
    }
}

template <typename TNum>
struct std::tuple_size<color::Color3<TNum>>
    : public integral_constant<std::size_t, color::Color3<TNum>::size()> {};

template <std::size_t I, typename TNum>
struct std::tuple_element<I, color::Color3<TNum>> {
    using type = TNum;
};
