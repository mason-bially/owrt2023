#pragma once

#include <optional>

#include "owrt.hpp"

namespace material
{
    template<typename TNum, typename TColorNum>
    struct MaterialResult {
        using Num = TNum;
        using ColorNum = TColorNum;

        vmath::Ray<Num> scattered;
        color::Color3<ColorNum> attenuation;
    };

    template<typename T, class TNum=T::Num, class TColorNum=T::ColorNum>
    concept Material = requires {
        typename T::Num;
        typename T::ColorNum;
        requires std::same_as<typename T::Num, TNum>;
        requires std::same_as<typename T::ColorNum, TColorNum>;
    } and requires (T m, vmath::Ray<TNum> const ray_in, object::HitRecord<TNum> const rec, color::Color3<TColorNum> attenuation) {
        { m.scatter(ray_in, rec, attenuation) } -> std::convertible_to<std::optional<MaterialResult<TNum, TColorNum>>>;
    };
}
