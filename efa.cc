#include <cairomm/cairomm.h>

int main(int argc, char* argv[]) {
    auto surface = Cairo::ImageSurface::create(
        Cairo::ImageSurface::Format::ARGB32,
        1600,
        600);
    return 0;
}