#include <iostream>
#include <iomanip>
#include <variant>

#include "owrt.hpp"

#include "sphere.hpp"

#include "camera.hpp"

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
    using Num = double;

    constexpr auto aspect_ratio = 16.0 / 9.0;
    constexpr int image_width = 400;
    constexpr int image_height = static_cast<int>(image_width / aspect_ratio);
    constexpr int samples_per_pixel = 100;

    // World

    object::HittableList<Num, ObjVariant> world;
    world.add<object::Sphere>({{0,0,-1}, 0.5});
    world.add<object::Sphere>({{0,-100.5,-1}, 100});

    // Camera

    common::RandomState rand;
    camera::SimpleCamera<Num> cam;

    // Render

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    for (auto j = image_height-1; j >= 0; --j)
    {
        std::cerr << "\rReamining: " 
            << std::setw(3) << int(j*1000.0/image_height) << "‰"
            << std::flush;
        for (auto i = 0; i < image_width; ++i)
        {
            Color3<Num> pixel_color;
            for (int s = 0; s < samples_per_pixel; ++s)
            {
                const auto u = Num(i + rand.num<double>()) / (image_width-1);
                const auto v = Num(j + rand.num<double>()) / (image_height-1);

                auto r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world);
            }
            pixel_color *= (Num(1) / samples_per_pixel);
            pixel_color = vmath::clamp(pixel_color, 0.0, 0.9999) * 256;
            std::cout << color_cast<Color3<uint8_t>>(pixel_color) << '\n';
        }
    }

    std::cerr << "\rComplete." << std::string(20, ' ') << "\n" << std::flush;
    return 0;
}
