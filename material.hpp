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

    template<typename T, typename TWorld=T::World>
    concept Material = WorldLike<TWorld> and std::same_as<TWorld, typename T::World>
        and requires (T m, typename TWorld::Ray const ray_in, object::HitRecord<TWorld> const rec) {
            { m.scatter(ray_in, rec) } -> std::convertible_to<std::optional<ScatterResult<TWorld>>>;
        };

    template<Material... TVariants>
    struct MaterialDispatch
        : public dispatch::DispatchGroup<TVariants...>
    {

    };

    template<WorldLike TWorld>
    struct AlwaysAbsorb
    {
        using World = TWorld;
        using Scatter = ScatterResult<World>;
        using Ray = typename TWorld::Ray;

        constexpr auto scatter(Ray const& in, auto const& hit_rec) -> std::optional<Scatter> {
            return {};
        }
    };
}
