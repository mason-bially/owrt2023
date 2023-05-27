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

    template<typename TNum>
    class SimpleCamera
    {
        public:
            using Num = TNum;
            using Loc3 = vmath::Loc3<Num>;
            using Vec3 = vmath::Vec3<Num>;
            using Ray = vmath::Ray<Num>;
            using HitRec = object::HitRecord<Num>;

        private:
            Loc3 _origin;
            Loc3 _cornerLowerLeft;
            Vec3 _horizontal;
            Vec3 _vertical;

        public:
            SimpleCamera()
            {
                auto aspect_ratio = 16.0 / 9.0;
                auto viewport_height = 2.0;
                auto viewport_width = aspect_ratio * viewport_height;
                auto focal_length = 1.0;

                _origin = Loc3 { 0, 0, 0 };
                _horizontal = Vec3 { viewport_width, 0.0, 0.0 };
                _vertical = Vec3 { 0.0, viewport_height, 0.0 };
                _cornerLowerLeft = _origin - _horizontal/TNum(2) - _vertical/TNum(2) - Vec3 { 0, 0, focal_length };
            }

            Ray get_ray(TNum u, TNum v) const {
                return Ray { _origin, _cornerLowerLeft + u*_horizontal + v*_vertical - _origin };
            }
    };
}
