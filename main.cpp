#include <iostream>
#include <fstream>
#include <iomanip>
#include <variant>
#include <functional>

#include "owrt.hpp"

#include "sphere.hpp"

#include "camera.hpp"

#define STBI_WRITE_NO_STDIO
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "vendor/stb_image_write.h"

using color::Color3;
using vmath::Ray;

template<dispatch::WorldLike TWorld>
using MatDispatch = material::MaterialDispatch<
    material::Absorb<TWorld>,
    material::Lambertian<TWorld>,
    material::Metal<TWorld>,
    material::Dielectric<TWorld>
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

template<class World>
void random_scene(object::HittableList<World>& world) {
    using Num = typename World::Num;
    using Loc = typename World::Loc;
    using Color = typename World::Color;

    using Lambertian = material::Lambertian<World>;
    using Metal = material::Metal<World>;
    using Dielectric = material::Dielectric<World>;

    common::RandomState rs;

    auto ground_material = Lambertian{{0.5, 0.5, 0.5}};
    world.template add<object::Sphere>({{0,-1000,0}, 1000, ground_material});

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = rand<Num>(rs);
            Loc center { a + 0.6*rand<Num>(rs), 0.2, b + 0.6*rand<Num>(rs) };

            if ((center - Loc{4, 0.2, 0}).length() > 0.9) {
                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = rand<Color>(rs) * rand<Color>(rs);
                    auto sphere_material = Lambertian{ albedo };
                    world.template add<object::Sphere>({center, 0.2, sphere_material});
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = rand<Color>(rs, 0.5, 1);
                    auto fuzz = rand<Num>(rs, 0, 0.5);
                    auto sphere_material = Metal{albedo, fuzz};
                    world.template add<object::Sphere>({center, 0.2, sphere_material});
                } else {
                    // glass
                    auto sphere_material = Dielectric{1.5};
                    world.template add<object::Sphere>({center, 0.2, sphere_material});
                }
            }
        }
    }

    auto material1 = Dielectric{1.5};
    world.template add<object::Sphere>({{0, 1, 0}, 1.0, material1});

    auto material2 = Lambertian{{0.4, 0.2, 0.1}};
    world.template add<object::Sphere>({{-4, 1, 0}, 1.0, material2});

    auto material3 = Metal{{0.7, 0.6, 0.5}, 0.0};
    world.template add<object::Sphere>({{4, 1, 0}, 1.0, material3});
}

template<typename... TArgs>
void invoke_function(void* ptr, TArgs... args)
{
    (*static_cast<std::function<void(TArgs...)>*>(ptr))(args...);
}

auto main() -> int
{
    // Types
    using Num = double;
    using ColorNum = double;

    using World = dispatch::World<
        Num, ColorNum,
        WorldConfig,
        MatDispatch, ObjDispatch>;

    // weird that one of these is needed...
    using Vec = typename World::Vec;
    using Loc = typename World::Loc;
    using Color = typename World::Color;

    // Image

    constexpr auto aspect_ratio = 16.0 / 9.0;
    constexpr int image_width = 800;
    constexpr int image_height = static_cast<int>(image_width / aspect_ratio);
    constexpr int samples_per_pixel = 100;
    constexpr int max_depth = 50;

    std::array<Color3<uint8_t>, image_width * image_height> image;

    // World

    common::RandomState rs;

    using Lambertian = material::Lambertian<World>;
    using Metal = material::Metal<World>;
    using Dielectric = material::Dielectric<World>;

    object::HittableList<World> world;
    /*
    random_scene<World>(world);
    */

    auto material_ground = Lambertian{{0.8, 0.8, 0.0}};
    auto material_center = Lambertian{{0.1, 0.2, 0.5}};
    auto material_left   = Dielectric{1.5};
    auto material_right  = Metal{{0.8, 0.6, 0.2}, 0.0};

    world.add<object::Sphere>({{ 0,-100.5,-1}, 100, material_ground});
    world.add<object::Sphere>({{ 0, 0,-1},  0.5, material_center});
    world.add<object::Sphere>({{-1, 0,-1},  0.5, material_left});
    world.add<object::Sphere>({{-1, 0,-1}, -0.4, material_left});
    world.add<object::Sphere>({{ 1, 0,-1},  0.5, material_right});

    // Camera
    Loc look_from {3,3,2};
    Loc look_to {0,0,-1};
    Num dist_to_focus = (look_from-Loc{-1,0,-1}).length();
    Num aperture = 0.5;

    camera::SimpleCamera<World> cam(
        look_from, look_to, Vec::Up,
        20, aspect_ratio,
        aperture, dist_to_focus);
    /*
    Loc look_from {13,2,3};
    Loc look_to {0,0,0};
    Num dist_to_focus = 10;
    Num aperture = 0.1;

    camera::SimpleCamera<World> cam(
        look_from, look_to, Vec::Up,
        20, aspect_ratio,
        aperture, dist_to_focus);
    */

    // Render

    for (auto j = image_height-1; j >= 0; --j)
    {
        std::cerr << "\rReamining: " 
            << std::setw(3) << int(j*1000.0/image_height) << "‰"
            << std::flush;
        for (auto i = 0; i < image_width; ++i)
        {
            Color pixel_color;
            for (int s = 0; s < samples_per_pixel; ++s)
            {
                const auto u = Num(i + rand<double>(rs)) / (image_width-1);
                const auto v = Num(j + rand<double>(rs)) / (image_height-1);

                auto r = cam.get_ray(u, v, rs);
                pixel_color += ray_color(r, world, rs, max_depth);
            }
            pixel_color *= (Num(1) / samples_per_pixel);
            pixel_color = map(pixel_color, [](auto v){ return std::sqrt(v); });
            pixel_color = clamp(pixel_color, 0.0, 0.9999) * 256;

            image[(image_height-j-1)*image_width + i] = color_cast<Color3<uint8_t>>(pixel_color);
        }
    }

    // Output

    {
        std::ofstream out_file("out.png");
        std::function<void(void*, int)> write_func = [&](void* data, int size) {
            out_file.write((char const*)data, size);
        };
        stbi_write_png_to_func(
            invoke_function<void*, int>, &write_func,
            image_width, image_height, 3,
            image.data(), image_width * 3
        );
        out_file.close();
    }


    std::cerr << "\rComplete." << std::string(20, ' ') << "\n" << std::flush;

    return 0;
}
