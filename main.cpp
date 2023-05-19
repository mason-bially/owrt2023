#include <iostream>
#include <iomanip>

auto main() -> int
{
    constexpr int image_width = 256;
    constexpr int image_height = 256;

    constexpr double color_conv = 255.999;

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    for (auto j = image_height-1; j >= 0; --j)
    {
        std::cerr << "\rReamining: " 
            << std::setw(3) << int(j*1000.0/image_height) << "‰"
            << std::flush;
        for (auto i = 0; i < image_width; ++i)
        {
            auto r = double(i) / (image_width-1);
            auto g = double(j) / (image_height-1);
            auto b = 0.25;

            auto ir = static_cast<int>(color_conv * r);
            auto ig = static_cast<int>(color_conv * g);
            auto ib = static_cast<int>(color_conv * b);

            std::cout << ir << ' ' << ig << ' ' << ib << '\n';
        }
    }

    std::cerr << "\rComplete." << std::string(20, ' ') << "\n" << std::flush;
    return 0;
}
