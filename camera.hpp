#pragma once

#include "owrt.hpp"

namespace camera
{
    template<typename T, class TNum=T::Num>
    concept Camera = requires {
        typename T::Num;
        requires std::same_as<typename T::Num, TNum>;
    } and requires (T c, TNum u, TNum v) {
        { c.get_ray(u, v) } -> std::convertible_to<vmath::Ray<TNum>>;
    };

    template<dispatch::WorldLike TWorld>
    class SimpleCamera
    {
        public:
            using Num = typename TWorld::Num;
            using Loc = typename TWorld::Loc;
            using Vec = typename TWorld::Vec;
            using Ray = typename TWorld::Ray;
            using HitRec = object::HitRecord<TWorld>;

        private:
            Loc _origin;
            Loc _cornerLowerLeft;
            Vec _horizontal;
            Vec _vertical;

        public:
            SimpleCamera(
                Num fov_vertical,
                Num aspect_ratio
            ) {
                auto theta = common::degrees_to_radians(fov_vertical);
                auto h = std::tan(theta/2);
                auto viewport_height = 2.0 * h;
                auto viewport_width = aspect_ratio * viewport_height;

                auto focal_length = 1.0;

                _origin = Loc{0, 0, 0};
                _horizontal = Vec{viewport_width, 0.0, 0.0};
                _vertical = Vec{0.0, viewport_height, 0.0};
                _cornerLowerLeft = _origin - _horizontal/Num(2) - _vertical/Num(2) - Vec{0, 0, focal_length};
            }

            Ray get_ray(Num u, Num v) const {
                return Ray{_origin, _cornerLowerLeft + u*_horizontal + v*_vertical - _origin};
            }
    };
}
