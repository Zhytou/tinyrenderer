#include <iostream>

#include "application.hpp"

int main(int argc, char** argv)
{
    tinyrenderer::Application app;
    try {
        app.load("../example/cornell-box/", "cornell-box.obj", "cornell-box.xml");
        app.run();
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}