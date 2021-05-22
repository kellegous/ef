#include <cairomm/cairomm.h>

int main(int argc, char* argv[]) {
    double width = 1600.0;
    double height = 600.0;

    auto surface = Cairo::ImageSurface::create(
        Cairo::ImageSurface::Format::ARGB32,
        width,
        height);
    auto context = Cairo::Context::create(surface);

    context->set_source_rgb(1.0, 1.0, 1.0);
    context->rectangle(0, 0, width, height);

    surface->write_to_png("out.png");

    return 0;
}