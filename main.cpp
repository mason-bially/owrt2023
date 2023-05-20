#include <iostream>
#include <iomanip>

#include "vmath.hpp"
#include "color.hpp"

#include "ray.hpp"

using color::Color3;

auto main() -> int
{
    constexpr int image_width = 256;
    constexpr int image_height = 256;

    vmath::Ray r;

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    for (auto j = image_height-1; j >= 0; --j)
    {
        std::cerr << "\rReamining: " 
            << std::setw(3) << int(j*1000.0/image_height) << "‰"
            << std::flush;
        for (auto i = 0; i < image_width; ++i)
        {
            Color3 pixel_color {
                double(i) / (image_width-1),
                double(j) / (image_height-1),
                0.25
            };

            std::cout << pixel_color << '\n';
        }
    }

    std::cerr << "\rComplete." << std::string(20, ' ') << "\n" << std::flush;
    return 0;
}
