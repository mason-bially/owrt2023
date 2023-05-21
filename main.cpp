#include <iostream>
#include <iomanip>
#include <variant>

#include "vmath.hpp"
#include "color.hpp"

#include "ray.hpp"
#include "sphere.hpp"

using color::Color3;
using vmath::Vec3;
using vmath::Loc3;
using vmath::Ray;

template<class TNum>
using ObjVariant = std::variant<
    object::Sphere<TNum>
>;

template<vmath::RayLike TRay>
auto hit_sphere(typename TRay::Loc const& center, typename TRay::Num radius, TRay const& r)
{
    auto oc = r.origin - center;
    auto a = r.direction.length_squared();
    auto half_b = dot(oc, r.direction);
    auto c = oc.length_squared() - radius*radius;
    auto discriminant = half_b*half_b - a*c;
    if (discriminant < 0)
        return -1.0;
    else
        return (-half_b - std::sqrt(discriminant)) / a;
}

template<typename Color = Color3<double>, object::Hittable World>
auto ray_color(vmath::RayLike auto const& r, World& world)
{
    using namespace common;

    if (auto hit = world.hit(r.span(0, infinity)); hit) {
        auto& n = hit->normal;
        return 0.5 * Color{n.x+1, n.y+1, n.z+1};
    }

    constexpr auto color_top = Color::White;
    constexpr auto color_bot = Color{0.5, 0.7, 1.0};

    auto unit_direction = unit_vector(r.direction);
    auto t = 0.5*(unit_direction.y + 1.0);
    return vmath::mix(color_top, color_bot, t);
}

auto main() -> int
{
    // Image

    constexpr auto aspect_ratio = 16.0 / 9.0;
    constexpr int image_width = 400;
    constexpr int image_height = static_cast<int>(image_width / aspect_ratio);

    // World

    object::HittableList<double, ObjVariant> world;
    world.add<object::Sphere>({{0,0,-1}, 0.5});
    world.add<object::Sphere>({{0,-100.5,-1}, 100});

    // Camera

    auto viewport_height = 2.0;
    auto viewport_width = aspect_ratio * viewport_height;
    auto focal_length = 1.0;

    auto origin = Loc3<double>::Origin;
    Vec3 horizontal { viewport_width, 0, 0 };
    Vec3 vertical { 0, viewport_height, 0 };
    auto lower_left_corner = origin - horizontal/2 - vertical/2 - Vec3{0, 0, focal_length};

    // Render

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
            auto pixel_color = ray_color(r, world);
            
            std::cout << color_cast<Color3<uint8_t>>(pixel_color) << '\n';
        }
    }

    std::cerr << "\rComplete." << std::string(20, ' ') << "\n" << std::flush;
    return 0;
}
