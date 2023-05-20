#pragma once

#include "ray.hpp"

namespace object
{
    template<typename TNum>
    struct HitRecord
    {
        using Num = TNum;
        using Loc = vmath::Loc3<Num>;
        using Vec = vmath::Vec3<Num>;

        Loc point;
        Vec normal;
        Num t;
    };

    template<typename TNum>
    class Hittable
    {
        public:
            using Num = TNum;
            using Ray = vmath::Ray<Num>;
            using HitRec = HitRecord<Num>;

        public:
            virtual ~Hittable() = default;

            virtual auto hit(Ray const& r, Num t_min, Num t_max, HitRec& rec) const -> bool = 0;
    };
}
