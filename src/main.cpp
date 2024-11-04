#include <iostream>

#include "application.hpp"

int main(int argc, char** argv)
{
    tinyrenderer::Application app(800, 600, "TinyRenderer");
    try {
        app.load("../scene/gun/gun.json", false);
        app.run(tinyrenderer::AppMode::RotatingCamera);
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}