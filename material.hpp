#pragma once

#include <optional>

#include "owrt.hpp"

namespace material
{
    using namespace dispatch;

    template<WorldLike TWorld>
    struct ScatterResult
    {
        using Ray = typename TWorld::Ray;
        using Color = typename TWorld::Color;

        Ray scattered;
        Color attenuation;
    };

    template<typename TWorld>
    using SamplerFunc = typename TWorld::Vec (*)(typename TWorld::Vec);

    template<typename T, typename TWorld=T::World>
    concept Material = WorldLike<TWorld> and std::same_as<TWorld, typename T::World>
        and requires (T const m, typename TWorld::Ray const ray_in, object::HitRecord<TWorld> const rec, SamplerFunc<TWorld> sampler) {
            { m.scatter(ray_in, rec, sampler) } -> std::convertible_to<std::optional<ScatterResult<TWorld>>>;
        };

    template<Material... TVariants>
    struct MaterialDispatch
        : public dispatch::DispatchGroup<TVariants...>
    {

    };

    template<WorldLike TWorld>
    struct Absorb
    {
        using World = TWorld;
        using Vec = typename TWorld::Vec;
        using Ray = typename TWorld::Ray;
        using Scatter = ScatterResult<World>;

        constexpr auto scatter(Ray const& in, auto const& hit_rec, std::invocable<Vec> auto& sampler) const -> std::optional<Scatter> {
            return std::nullopt;
        }
    };

    template<WorldLike TWorld>
    struct Lambertian
    {
        using World = TWorld;
        using Vec = typename TWorld::Vec;
        using Ray = typename TWorld::Ray;
        using Color = typename TWorld::Color;
        using Scatter = ScatterResult<World>;

        Color albedo;

        constexpr auto scatter(Ray const& in, auto const& hit_rec, std::invocable<Vec> auto& sampler) const -> std::optional<Scatter> {
            auto scatter_dir = sampler(hit_rec.normal);
            if (near_zero(scatter_dir)) scatter_dir = hit_rec.normal;

            return Scatter {
                Ray{hit_rec.point, scatter_dir},
                albedo
            };
        }
    };

    template<WorldLike TWorld>
    struct Metal
    {
        using World = TWorld;
        using Vec = typename TWorld::Vec;
        using Ray = typename TWorld::Ray;
        using Color = typename TWorld::Color;
        using Scatter = ScatterResult<World>;

        Color albedo;

        constexpr auto scatter(Ray const& in, auto const& hit_rec, std::invocable<Vec> auto& sampler) const -> std::optional<Scatter> {
            auto reflected = reflect(unit_vector(in.direction), hit_rec.normal);
            if (dot(reflected, hit_rec.normal) > 0)
                return Scatter {
                    Ray{hit_rec.point, reflected},
                    albedo
                };
            else
                return std::nullopt;
        }
    };
}
