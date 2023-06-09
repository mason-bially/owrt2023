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
    using common::map;
    using common::mix;
    using common::clamp;
    using common::rand;
    using common::near_zero;

    /* Concepts */

    template<typename T>
    concept IsAffine = T::IsAffineSpace();

    template<typename T>
    concept Tup3LikePart0 = common::Tup3Like<T> and requires {
            { T::IsAffineSpace() } -> std::convertible_to<bool>;
        } and requires(T n) {
            { n.length() } -> std::convertible_to<typename T::Num>;
            { affine(n) } -> IsAffine;
        };

    template<typename T>
    concept Tup3Like = Tup3LikePart0<T> and requires(T n) {
            { affine(n) } -> common::Tup3Like;
    };

    template<typename T> concept Tup3Affine = Tup3Like<T> and IsAffine<T>;
    template<typename T> concept Tup3NonAffine = Tup3Like<T> and !IsAffine<T>;

    /* Classes */

    template<typename TActual, typename TNum>
    struct Tup3Base
    {
        using Num = TNum;

        Num x = 0;
        Num y = 0;
        Num z = 0;

        /* no constructors to not interfere with tuple-ification */

        constexpr auto operator-() const { return TActual {-x, -y, -z}; }

        /* prefer `std::get<i>(vec)` for compile time access */

        static constexpr auto size() { return 3; }
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

        constexpr auto operator+=(Tup3Affine auto vec)
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
    constexpr auto& get(Tup3Base<TActual, TNum>& n)
    {
        if constexpr (I == 0) return n.x;
        else if constexpr (I == 1) return n.y;
        else if constexpr (I == 2) return n.z;
        else static_assert("Get Index out of Range");
    }
    template <unsigned I, typename TActual, typename TNum>
    constexpr auto get(Tup3Base<TActual, TNum> const& n)
    {
        if constexpr (I == 0) return n.x;
        else if constexpr (I == 1) return n.y;
        else if constexpr (I == 2) return n.z;
        else static_assert("Get Index out of Range");
    }


    template<class TNum = double>
    struct Vec3
        : public Tup3Base<Vec3<TNum>, TNum>
    {
        using Num = TNum;

        using Self = Vec3<TNum>;
        using Base = Tup3Base<Vec3<TNum>, TNum>;
        using Base::x;
        using Base::y;
        using Base::z;

        static constexpr Num Num0 = 0;
        static constexpr Num Num1 = Num(1);

        static constexpr Self Up        {  Num0, +Num1,  Num0 };
        static constexpr Self Down      {  Num0, -Num1,  Num0 };
        static constexpr Self Forward   {  Num0,  Num0, +Num1 };
        static constexpr Self Backward  {  Num0,  Num0, -Num1 };
        static constexpr Self Left      { -Num1,  Num0,  Num0 };
        static constexpr Self Right     { +Num1,  Num0,  Num0 };

        static constexpr auto IsAffineSpace() { return true; }
    };
    template<class NX, class NY, class NZ> Vec3 (NX, NY, NZ) -> Vec3<common::numeric_promote_t<NX, NY, NZ>>;
    template<class Num> constexpr auto affine (Vec3<Num> n) { return n; }


    template<class Num = double>
    struct Loc3
        : public Tup3Base<Loc3<Num>, Num>
    {
        using Self = Loc3<Num>;
        using Base = Tup3Base<Self, Num>;
        using Base::x;
        using Base::y;
        using Base::z;

        static constexpr Self Origin { 0, 0, 0 };

        static constexpr auto IsAffineSpace() { return false; }

        constexpr explicit operator Vec3<Num>() { return Vec3 { x, y, z }; }
    };
    template<class NX, class NY, class NZ> Loc3 (NX, NY, NZ) -> Loc3<common::numeric_promote_t<NX, NY, NZ>>;
    template<class Num> constexpr auto affine (Loc3<Num> n) { return (Vec3<Num>) n; }

    /* Utility Functions */

    template<Tup3Like TN3A, Tup3Like TN3B>
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
    template<Tup3Like TN3A, Tup3Like TN3B>
    constexpr auto operator-(TN3A const& u, TN3B const& v)
    {
        if constexpr (IsAffine<TN3B>)
            return TN3A { u.x - v.x, u.y - v.y, u.z - v.z };
        else if constexpr (IsAffine<TN3A> and !IsAffine<TN3B>)
            return TN3B { u.x - v.x, u.y - v.y, u.z - v.z };
        else
            return affine(u) - affine(v);
    }

    template<Tup3Affine TN3>
    constexpr auto operator*(typename TN3::Num t, TN3 const& v)
    {
        return TN3 { t*v.x, t*v.y, t*v.z };
    }

    template<Tup3Affine TN3>
    constexpr auto operator*(TN3 const& v, typename TN3::Num t)
    {
        return t * v;
    }

    template<Tup3Affine TN3>
    constexpr auto operator/(TN3 const& v, typename TN3::Num t)
    {
        return (1/t) * v;
    }

    template<Tup3Affine TN3>
    constexpr auto dot(TN3 const& u, TN3 const& v)
    {
        auto [ux, uy, uz] = u; auto [vx, vy, vz] = v;
        return ux * vx + uy * vy + uz * vz;
    }

    template<Tup3Affine TN3>
    constexpr auto cross(TN3 const& u, TN3 const& v)
    {
        auto [ux, uy, uz] = u; auto [vx, vy, vz] = v;
        return TN3 {
            uy * vz - uz * vy,
            uz * vx - ux * vz,
            ux * vy - uy * vx
        };
    }

    constexpr auto unit_vector(Tup3Like auto const& v) { return affine(v) / v.length(); }

    template<Tup3Affine TN3>
    constexpr auto reflect(TN3 const& v, TN3 const& n) {
        return v - 2*dot(v,n)*n;
    }

    template<Tup3Affine TN3>
    constexpr auto refract(TN3 const& uv, TN3 const& n, typename TN3::Num etai_over_etat) {
        using Num = typename TN3::Num;

        auto cos_theta = std::min(dot(-uv, n), Num(1));
        auto r_out_perp = etai_over_etat * (uv + cos_theta*n);
        auto r_out_parallel = -std::sqrt(std::abs(Num(1) - r_out_perp.length_squared())) * n;
        return r_out_perp + r_out_parallel;
    }

    template<Tup3Like TN3>
    constexpr auto rand_in_sphere(common::RandomState& rs) {
        using Num = typename TN3::Num;
        while (true) {
            auto p = rand<TN3>(rs, Num(-1), Num(1));
            if (p.length_squared() >= 1) continue;
            return p;
        }
    }

    template<Tup3Like TN3>
    constexpr auto rand_in_disk(common::RandomState& rs) {
        using Num = typename TN3::Num;
        while (true) {
            auto p = TN3 { rand<Num>(rs, -1, 1), rand<Num>(rs, -1, 1), 0 };
            if (p.length_squared() >= 1) continue;
            return p;
        }
    }

    template<Tup3Like TN3>
    constexpr auto rand_unit_vector(common::RandomState& rs) {
        return unit_vector(rand_in_sphere<TN3>(rs));
    }

    template<Tup3Like TN3>
    constexpr auto rand_in_hemisphere(TN3 const& normal, common::RandomState& rs) {
        auto in_sphere = rand_in_sphere<TN3>(rs);
        if (dot(in_sphere, normal) > 0.0) // In the same hemisphere as the normal
            return in_sphere;
        else
            return -in_sphere;
    }
}

template <typename TNum>
struct std::tuple_size<vmath::Vec3<TNum>>
    : public integral_constant<std::size_t, vmath::Vec3<TNum>::size()> {};

template <std::size_t I, typename TNum>
struct std::tuple_element<I, vmath::Vec3<TNum>> {
    using type = TNum;
};

template <typename TNum>
struct std::tuple_size<vmath::Loc3<TNum>>
    : public integral_constant<std::size_t, vmath::Loc3<TNum>::size()> {};

template <std::size_t I, typename TNum>
struct std::tuple_element<I, vmath::Loc3<TNum>> {
    using type = TNum;
};
