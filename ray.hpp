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

    template<class TNum = double>
    struct Ray
    {
        using Num = TNum;
        using Loc = Loc3<Num>;
        using Vec = Vec3<Num>;

        Loc origin;
        Vec direction;

        constexpr auto at(Num t) const { return origin + t*direction; }
    };
}
