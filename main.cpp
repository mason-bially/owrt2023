#include <iostream>
#include <iomanip>

#include "vmath.hpp"
#include "color.hpp"

#include "ray.hpp"

using color::Color3;
using vmath::Vec3;
using vmath::Loc3;

template<typename TNum = double>
auto ray_color(vmath::Ray<TNum> const& r)
{
    constexpr auto color_top = Color3 { 1.0, 1.0, 1.0 };
    constexpr auto color_bot = Color3 { 0.5, 0.7, 1.0 };

    auto unit_direction = vmath::unit_vector(r.direction);
    auto t = 0.5*(unit_direction.y + 1.0);
    return vmath::mix(color_top, color_bot, t);
}

auto main() -> int
{
    // Image
    constexpr auto aspect_ratio = 16.0 / 9.0;
    constexpr int image_width = 400;
    constexpr int image_height = static_cast<int>(image_width / aspect_ratio);

    // Camera

    auto viewport_height = 2.0;
    auto viewport_width = aspect_ratio * viewport_height;
    auto focal_length = 1.0;

    Loc3 origin;
    Vec3 horizontal { viewport_width, 0, 0 };
    Vec3 vertical { 0, viewport_height, 0 };
    auto lower_left_corner = origin - horizontal/2 - vertical/2 - Vec3{0, 0, focal_length};

    // Render

    vmath::Ray r;

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    for (auto j = image_height-1; j >= 0; --j)
    {
        std::cerr << "\rReamining: " 
            << std::setw(3) << int(j*1000.0/image_height) << "‰"
            << std::flush;
        for (auto i = 0; i < image_width; ++i)
        {
            const auto u = double(i) / (image_width-1);
            const auto v = double(j) / (image_height-1);
            
            vmath::Ray r { origin, lower_left_corner + u*horizontal + v*vertical - origin };
            Color3 pixel_color = ray_color(r);
            
            std::cout << pixel_color << '\n';
        }
    }

    std::cerr << "\rComplete." << std::string(20, ' ') << "\n" << std::flush;
    return 0;
}
