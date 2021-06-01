#include "charge.h"

#include <cairomm/cairomm.h>
#include <iostream>
#include <libgen.h>
#include <memory>
#include <random>
#include <vector>

namespace {

void CreateCharges(
    std::vector<std::shared_ptr<Charge>>* dst,
    double w,
    double h,
    size_t n) {
    std::default_random_engine rng;
    std::uniform_real_distribution<double> hdist(0, h);
    std::uniform_real_distribution<double> cdist(-10, 10);

    dst->clear();
    dst->reserve(n);
    double dx = w / n;
    for (int i = 0; i < n; i++) {
        std::shared_ptr<Charge> c(
            new Charge(i*dx + dx/2, hdist(rng), cdist(rng))
        );
        dst->push_back(c);
    }
}

}

int main(int argc, char* argv[]) {
    double width = 1600.0;
    double height = 600.0;

    std::vector<std::shared_ptr<Charge>> charges;
    CreateCharges(&charges, width, height, 20);

    auto surface = Cairo::ImageSurface::create(
        Cairo::ImageSurface::Format::ARGB32,
        width,
        height);
    auto context = Cairo::Context::create(surface);

    context->set_source_rgb(0.0, 0.0, 0.0);
    context->rectangle(0, 0, width, height);
    context->fill();

    context->set_source_rgb(1.0, 1.0, 1.0);
    for (auto it = charges.begin(); it != charges.end(); it++) {
        auto c = *it;
        auto pt = c->point();
        context->begin_new_path();
        context->arc(pt.i(), pt.j(), 3, 0, 2 * M_PI);
        context->fill();
    }

    std::string dst(basename(argv[0]));
    dst.append(".png");
    surface->write_to_png(dst.c_str());

    return 0;
}