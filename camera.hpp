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
                Loc look_from,
                Loc look_to,
                Vec upvec,
                Num fov_vertical,
                Num aspect_ratio
            ) {
                auto theta = common::degrees_to_radians(fov_vertical);
                auto h = std::tan(theta/2);
                auto viewport_height = 2.0 * h;
                auto viewport_width = aspect_ratio * viewport_height;

                auto w = unit_vector(look_from - look_to);
                auto u = unit_vector(cross(upvec, w));
                auto v = cross(w, u);

                _origin = look_from;
                _horizontal = viewport_width * u;
                _vertical = viewport_height * v;
                _cornerLowerLeft = _origin - _horizontal/Num(2) - _vertical/Num(2) - w;
            }

            Ray get_ray(Num s, Num t) const {
                return Ray{_origin, _cornerLowerLeft + s*_horizontal + t*_vertical - _origin};
            }
    };
}
