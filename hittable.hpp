#pragma once

#include <vector>

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
        bool front_face;

        constexpr void set_face_normal(vmath::RayLike auto const& r, Vec const& outward_normal)
        {
            front_face = dot(r.direction, outward_normal) < 0;
            normal = front_face ? outward_normal : -outward_normal;
        }
    };

    template<typename T, class TNum=T::Num>
    concept Hittable = requires {
        typename T::Num;
        std::same_as<typename T::Num, TNum>;
    } and requires (T h, vmath::Ray<TNum> r, TNum min, TNum max, HitRecord<TNum> hitrec) {
        { h.hit(r, min, max, hitrec) } -> std::convertible_to<bool>;
    };

    template<class TNum, template<class> typename THittableVariant>
    struct HittableList
    {
        using Num = TNum;
        using HittableVariant = THittableVariant<TNum>;
        using Ray = vmath::Ray<TNum>;
        using HitRec = HitRecord<TNum>;

        std::vector<HittableVariant> objects;

        template<template<class> typename THittable>
        constexpr void add(THittable<Num>&& hittable)
        {
            objects.emplace_back(std::forward<THittable<Num>&&>(hittable));
        }

        constexpr auto hit(vmath::RayLike auto const& r, TNum t_min, TNum t_max, HitRec& rec) const
        {
            HitRec temp_rec;
            bool hit_anything = false;
            auto closest_so_far = t_max;

            for (const auto& object : objects) {
                bool hit = std::visit([&](auto&& o) { return o.hit(r, t_min, closest_so_far, temp_rec); }, object);
                if (hit) {
                    hit_anything = true;
                    closest_so_far = temp_rec.t;
                    rec = temp_rec;
                }
            }

            return hit_anything;
        }
    };
}
