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

template<dispatch::WorldLike TWorld>
using MatDispatch = material::MaterialDispatch<
    material::AlwaysAbsorb<TWorld>
>;

template<dispatch::WorldLike TWorld>
using ObjDispatch = object::HittableDispatch<
    object::Sphere<TWorld>
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

template<typename Color = Color3<double>, vmath::RayLike Ray>
auto ray_color(Ray const& r, object::Hittable auto& world, std::invocable<typename Ray::Vec> auto& sampler, int depth) -> Color
{
    using namespace common;

    if (depth <= 0)
        return Color::Black;

    if (auto hit = world.hit(r.span(0.001, infinity)); hit)
    {
        auto target = hit->point + sampler(hit->normal);
        return 0.5 * ray_color(Ray { hit->point, target - hit->point }, world, sampler, depth-1);
    }

    constexpr auto color_top = Color::White;
    constexpr auto color_bot = Color{0.5, 0.7, 1.0};

    auto unit_direction = unit_vector(r.direction);
    auto t = 0.5*(unit_direction.y + 1.0);
    return mix(color_top, color_bot, t);
}

auto main() -> int
{
    // Types
    using Num = double;
    using ColorNum = double;

    using World = dispatch::World<
        Num, ColorNum,
        MatDispatch, ObjDispatch>;

    using Vec = typename World::Vec;

    // Image

    constexpr auto aspect_ratio = 16.0 / 9.0;
    constexpr int image_width = 400;
    constexpr int image_height = static_cast<int>(image_width / aspect_ratio);
    constexpr int samples_per_pixel = 100;
    constexpr int max_depth = 50;

    // Samplers

    common::RandomState rs;
    auto sample_sphere = [&](Vec normal) { return normal + vmath::rand_in_sphere<Vec>(rs); };
    auto sample_unit_vector = [&](Vec normal) { return normal + vmath::rand_unit_vector<Vec>(rs); };
    auto sample_hemisphere = [&](Vec normal) { return vmath::rand_in_hemisphere<Vec>(normal, rs); };
    auto& sampler = sample_hemisphere;

    // World

    object::HittableList<World> world;
    world.add<object::Sphere>({{0,0,-1}, 0.5});
    world.add<object::Sphere>({{0,-100.5,-1}, 100});

    // Camera

    camera::SimpleCamera<World> cam;

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
                const auto u = Num(i + rand<double>(rs)) / (image_width-1);
                const auto v = Num(j + rand<double>(rs)) / (image_height-1);

                auto r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, sampler, max_depth);
            }
            pixel_color *= (Num(1) / samples_per_pixel);
            pixel_color = map(pixel_color, [](auto v){ return std::sqrt(v); });
            pixel_color = clamp(pixel_color, 0.0, 0.9999) * 256;
            std::cout << color_cast<Color3<uint8_t>>(pixel_color) << '\n';
        }
    }

    std::cerr << "\rComplete." << std::string(20, ' ') << "\n" << std::flush;
    return 0;
}
