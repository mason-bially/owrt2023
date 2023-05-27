#pragma once

#include <vector>
#include <optional>

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
        requires std::same_as<typename T::Num, TNum>;
    } and requires (T h, vmath::RaySegment<TNum> seg) {
        { h.hit(seg) } -> std::convertible_to<std::optional<HitRecord<TNum>>>;
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

        constexpr auto hit(vmath::RaySegLike auto seg) const
        {
            std::optional<HitRec> rec;

            for (const auto& object : objects)
            {
                auto hit = std::visit([&](auto&& o) { return o.hit(seg); }, object)
                    //.and_then([&](auto const& hit) { seg.t_max = hit.t; return hit; })
                    //.or_else(*rec)
                    ;
                // TODO temp untill full c++23
                if (hit) {
                    rec = hit;
                    seg.t_max = rec->t;
                }
            }

            return rec;
        }
    };
}
