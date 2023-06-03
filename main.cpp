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
    material::Absorb<TWorld>,
    material::Lambertian<TWorld>,
    material::Metal<TWorld>
>;

template<dispatch::WorldLike TWorld>
using ObjDispatch = object::HittableDispatch<
    object::Sphere<TWorld>
>;

template<typename Color = Color3<double>, vmath::RayLike Ray>
auto ray_color(Ray const& r, object::Hittable auto& world, common::RandomState& rs, int depth) -> Color
{
    using namespace common;

    if (depth <= 0)
        return Color::Black;

    if (auto hit = world.hit(r.span(0.001, infinity)); hit)
    {
        auto scatter_dispatch = [&](auto&& m) { return m.scatter(r, *hit, rs); };
        if (auto scatter = std::visit(scatter_dispatch, *hit->material); scatter)
        {
            return scatter->attenuation * ray_color(scatter->scattered, world, rs, depth-1);
        }
        return Color::Black;
    }

    constexpr auto color_top = Color::White;
    constexpr auto color_bot = Color{0.5, 0.7, 1.0};

    auto unit_direction = unit_vector(r.direction);
    auto t = 0.5*(unit_direction.y + 1.0);
    return mix(color_top, color_bot, t);
}

template<typename TWorld>
struct WorldConfig
{
    using Vec = typename TWorld::Vec;

    static constexpr auto sample_sphere(Vec normal, common::RandomState& rs) { return normal + vmath::rand_in_sphere<Vec>(rs); }
    static constexpr auto sample_unit_vector(Vec normal, common::RandomState& rs) { return normal + vmath::rand_unit_vector<Vec>(rs); }
    static constexpr auto sample_hemisphere(Vec normal, common::RandomState& rs) { return vmath::rand_in_hemisphere<Vec>(normal, rs); }
    static constexpr auto lambertian_sampler(Vec normal, common::RandomState& rs) { return sample_hemisphere(normal, rs); }
};

auto main() -> int
{
    // Types
    using Num = double;
    using ColorNum = double;

    using World = dispatch::World<
        Num, ColorNum,
        WorldConfig,
        MatDispatch, ObjDispatch>;

    using Vec = typename World::Vec; // weird that this is required

    // Image

    constexpr auto aspect_ratio = 16.0 / 9.0;
    constexpr int image_width = 400;
    constexpr int image_height = static_cast<int>(image_width / aspect_ratio);
    constexpr int samples_per_pixel = 100;
    constexpr int max_depth = 50;

    // World

    common::RandomState rs;

    using Lambertian = material::Lambertian<World>;
    using Metal = material::Metal<World>;

    auto material_ground = Lambertian{{0.8, 0.8, 0.0}};
    auto material_center = Lambertian{{0.7, 0.3, 0.3}};
    auto material_left   = Metal{{0.8, 0.8, 0.8}, 0.3};
    auto material_right  = Metal{{0.8, 0.6, 0.2}, 1.0};

    object::HittableList<World> world;
    world.add<object::Sphere>({{ 0,-100.5,-1}, 100, material_ground});
    world.add<object::Sphere>({{ 0, 0,-1}, 0.5, material_center});
    world.add<object::Sphere>({{-1, 0,-1}, 0.5, material_left});
    world.add<object::Sphere>({{ 1, 0,-1}, 0.5, material_right});

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
                pixel_color += ray_color(r, world, rs, max_depth);
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
