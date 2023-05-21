#pragma once

#include "hittable.hpp"

namespace object
{
    template<typename TNum>
    class Sphere
    {
        public:
            using Num = TNum;
            using Loc = vmath::Loc3<Num>;
            using Ray = vmath::Ray<Num>;
            using HitRec = HitRecord<Num>;

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
