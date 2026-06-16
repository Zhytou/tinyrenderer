#include <iostream>

#include "application.hpp"
#include "utils.hpp"

std::string scene = "../asset/scene/raw/sponza.json";

int main(int argc, char** argv) {
    try {
        tinyrenderer::Application app(1200, 900, "TinyRenderer");
        app.load(scene);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}