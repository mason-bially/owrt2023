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
        and requires (T const m, typename TWorld::Ray const ray_in, object::HitRecord<TWorld> const rec, common::RandomState rs) {
            { m.scatter(ray_in, rec, rs) } -> std::convertible_to<std::optional<ScatterResult<TWorld>>>;
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
        using Vec = typename World::Vec;
        using Ray = typename World::Ray;
        using Scatter = ScatterResult<World>;

        constexpr auto scatter(Ray const& in, auto const& hit_rec, common::RandomState& rs) const -> std::optional<Scatter> {
            return std::nullopt;
        }
    };

    template<WorldLike TWorld>
    struct Lambertian
    {
        using World = TWorld;
        using Vec = typename World::Vec;
        using Ray = typename World::Ray;
        using Color = typename World::Color;
        using Scatter = ScatterResult<World>;

        Color albedo;

        constexpr auto scatter(Ray const& in, auto const& hit_rec, common::RandomState& rs) const -> std::optional<Scatter> {
            auto scatter_dir = World::Config::lambertian_sampler(hit_rec.normal, rs);
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
        using Num = typename World::Num;
        using Vec = typename World::Vec;
        using Ray = typename World::Ray;
        using Color = typename World::Color;
        using Scatter = ScatterResult<World>;

        Color albedo;
        Num fuzz;

        constexpr auto scatter(Ray const& in, auto const& hit_rec, common::RandomState& rs) const -> std::optional<Scatter> {
            auto reflected = reflect(unit_vector(in.direction), hit_rec.normal) + fuzz*vmath::rand_in_sphere<Vec>(rs);
            if (dot(reflected, hit_rec.normal) > 0)
                return Scatter {
                    Ray{hit_rec.point, reflected},
                    albedo
                };
            else
                return std::nullopt;
        }
    };

    template<WorldLike TWorld>
    struct Dielectric
    {
        using World = TWorld;
        using Num = typename World::Num;
        using Vec = typename World::Vec;
        using Ray = typename World::Ray;
        using Color = typename World::Color;
        using Scatter = ScatterResult<World>;

        Num ir; // Index of Refraction

        constexpr auto scatter(Ray const& in, auto const& hit_rec, common::RandomState& rs) const -> std::optional<Scatter> {
            constexpr auto attenuation = Color::White;
            Num refraction_ratio = hit_rec.front_face ? (1.0/ir) : ir;

            Vec unit_direction = unit_vector(in.direction);
            Num cos_theta = std::min(dot(-unit_direction, hit_rec.normal), 1.0);
            Num sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

            bool cannot_refract = refraction_ratio * sin_theta > 1.0;
            Vec direction;
            if (cannot_refract || reflectance(cos_theta, refraction_ratio) > rand<Num>(rs))
                direction = reflect(unit_direction, hit_rec.normal);
            else
                direction = refract(unit_direction, hit_rec.normal, refraction_ratio);

            return Scatter {
                Ray{hit_rec.point, direction},
               attenuation
            };
        }

    private:
        static Num reflectance(Num cosine, Num ref_idx) {
            // Use Schlick's approximation for reflectance.
            auto r0 = (1-ref_idx) / (1+ref_idx);
            r0 = r0*r0;
            return r0 + (1-r0)*std::pow((1 - cosine),5);
        }
    };
}
