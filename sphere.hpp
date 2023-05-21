#pragma once

#include "hittable.hpp"

namespace object
{
    template<typename TNum>
    class Sphere
        : public Hittable<TNum>
    {
        public:
            using Num = TNum;
            using Loc = vmath::Loc3<Num>;
            using Ray = vmath::Ray<Num>;
            using HitRec = HitRecord<Num>;

            Loc center;
            Num radius;

        public:
            virtual ~Sphere() = default;

            virtual bool hit(Ray const& r, Num t_min, Num t_max, HitRec& rec) const override;
    };

    template<typename TNum>
    bool Sphere<TNum>::hit(Ray const& r, Num t_min, Num t_max, HitRec& rec) const
    {
        auto oc = r.origin - center;
        auto a = r.direction.length_squared();
        auto half_b = dot(oc, r.direction);
        auto c = oc.length_squared() - radius*radius;

        auto discriminant = half_b*half_b - a*c;
        if (discriminant < 0) return false;
        auto sqrtd = sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (-half_b - sqrtd) / a;
        if (root < t_min || t_max < root) {
            root = (-half_b + sqrtd) / a;
            if (root < t_min || t_max < root)
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        auto outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);

        return true;
    }
}
