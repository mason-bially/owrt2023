#pragma once

#include "vmath.hpp"

namespace vmath
{
    template<class TNum = double>
    struct Ray
    {
        Loc3<TNum> origin;
        Vec3<TNum> direction;

        constexpr auto at(TNum t) const { return origin + t*direction; }
    };
}
