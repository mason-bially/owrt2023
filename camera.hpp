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

            Vec _u, _v, _w;
            Num _lens_radius;

        public:
            SimpleCamera(
                Loc look_from,
                Loc look_to,
                Vec upvec,
                Num fov_vertical,
                Num aspect_ratio,
                Num aperture,
                Num focus_dist
            ) {
                auto theta = common::degrees_to_radians(fov_vertical);
                auto h = std::tan(theta/2);
                auto viewport_height = 2.0 * h;
                auto viewport_width = aspect_ratio * viewport_height;

                _w = unit_vector(look_from - look_to);
                _u = unit_vector(cross(upvec, _w));
                _v = cross(_w, _u);

                _origin = look_from;
                _horizontal = focus_dist * viewport_width * _u;
                _vertical = focus_dist * viewport_height * _v;
                _cornerLowerLeft = _origin - _horizontal/Num(2) - _vertical/Num(2) - focus_dist*_w;

                _lens_radius = aperture / 2;
            }

            Ray get_ray(Num s, Num t, common::RandomState& rs) const {
                auto rd = _lens_radius * vmath::rand_in_disk<Vec>(rs);
                auto offset = _u * rd.x + _v * rd.y;
                return Ray { 
                    _origin + offset,
                    _cornerLowerLeft + s*_horizontal + t*_vertical - _origin - offset
                };
            }
    };
}
