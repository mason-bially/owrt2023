#pragma once

#include <vector>
#include <optional>

#include "ray.hpp"

namespace object
{
    using namespace dispatch;

    template<WorldLike TWorld>
    struct HitRecord
    {
        using World = TWorld;
        using Num = typename World::Num;
        using Loc = typename World::Loc;
        using Vec = typename World::Vec;
        using MatVar = typename World::MatVar;

        Loc point;
        Vec normal;
        MatVar material;
        Num t;
        bool front_face;

        constexpr void set_face_normal(vmath::RayLike auto const& r, Vec const& outward_normal)
        {
            front_face = dot(r.direction, outward_normal) < 0;
            normal = front_face ? outward_normal : -outward_normal;
        }
    };

    template<typename T, typename TWorld=T::World>
    concept Hittable = WorldLike<TWorld> and std::same_as<TWorld, typename T::World>
        and requires (T h, vmath::RaySegment<typename TWorld::Num> seg) {
            { h.hit(seg) } -> std::convertible_to<std::optional<HitRecord<TWorld>>>;
        };

    template<Hittable... TVariants>
    struct HittableDispatch
        : public DispatchGroup<TVariants...> {
        
    };

    template<WorldLike TWorld>
    struct HittableList
    {
        using World = TWorld;
        using ObjVar = typename TWorld::ObjVar;
        using Ray = typename TWorld::Ray;
        using HitRec = HitRecord<TWorld>;

        std::vector<ObjVar> objects;

        template<template<class> typename THittable>
        constexpr void add(THittable<TWorld>&& hittable)
        {
            objects.emplace_back(std::forward<THittable<TWorld>&&>(hittable));
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
