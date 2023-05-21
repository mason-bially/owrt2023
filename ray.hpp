#pragma once

#include "vmath.hpp"

namespace vmath
{
    template<typename T>
    concept RayLike = requires(T r)
    {
        Tup3NonAffine<decltype(r.origin)>;
        Tup3Affine<decltype(r.direction)>;
    }
    and requires(T r, typename T::Num t)
    {
        Tup3NonAffine<decltype(r.at(t))>;
    };

    template<class TNum=double>
    struct Ray
    {
        using Num = TNum;
        using Self = Ray<Num>;
        using Loc = Loc3<Num>;
        using Vec = Vec3<Num>;

        Loc origin;
        Vec direction;

        constexpr auto at(Num t) const { return origin + t*direction; }

        constexpr auto segment(Num t_min, Num t_max) const;
        constexpr auto span(Num t_min, Num t_max) const;
    };

    template<typename T>
    concept RaySegLike = requires {
        typename T::Num;
    } and requires(T s)
    {
        RayLike<std::remove_cvref_t<decltype(s.ray)>>;
        std::same_as<decltype(s.t_min), typename T::Num>;
        std::same_as<decltype(s.t_max), typename T::Num>;
    };

    template<class TNum=double, typename TRay=Ray<TNum>>
        requires RayLike<std::remove_cvref_t<TRay>>
    struct RaySegment
    {
        using Num = TNum;
        using Ray = std::remove_cv_t<TRay>;

        TRay ray;
        Num t_min;
        Num t_max;

        constexpr auto with_max(Num t_max) const { return RaySegment { ray, t_min, t_max }; }
        constexpr auto with_min(Num t_min) const { return RaySegment { ray, t_min, t_max }; }
    };

    template<class TNum>
    constexpr auto Ray<TNum>::segment(Num t_min, Num t_max) const
    {
        return RaySegment<TNum, Ray> { *this, t_min, t_max };
    }
    template<class TNum>
    constexpr auto Ray<TNum>::span(Num t_min, Num t_max) const
    {
        return RaySegment<TNum, Ray const&> { *this, t_min, t_max };
    }
}
