#pragma once

#include "hittable.hpp"

namespace object
{
    template<typename TWorld>
    class Sphere
    {
        public:
            using World = TWorld;
            using Num = typename World::Num;
            using Loc = typename World::Loc;
            using Ray = typename World::Ray;
            using HitRec = HitRecord<World>;

            Loc center;
            Num radius;

            constexpr auto hit(vmath::RaySegLike auto seg) const -> std::optional<HitRec>
            {
                auto const& [r, t_min, t_max] = seg;

                auto oc = r.origin - center;
                auto a = r.direction.length_squared();
                auto half_b = dot(oc, r.direction);
                auto c = oc.length_squared() - radius*radius;

                auto discriminant = half_b*half_b - a*c;
                if (discriminant < 0) return std::nullopt;
                auto sqrtd = sqrt(discriminant);

                // Find the nearest root that lies in the acceptable range.
                auto root = (-half_b - sqrtd) / a;
                if (root < t_min || t_max < root) {
                    root = (-half_b + sqrtd) / a;
                    if (root < t_min || t_max < root)
                        return std::nullopt;
                }

                HitRec rec;
                rec.t = root;
                rec.point = r.at(rec.t);
                auto outward_normal = (rec.point - center) / radius;
                rec.set_face_normal(r, outward_normal);

                return rec;
            }
    };
}
