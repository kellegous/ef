#include "efa.h"

#include "charge.h"
#include "color.h"
#include "color_client.h"
#include "ef.h"
#include "seed.h"
#include "vec2.h"

#include <cairomm/cairomm.h>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

namespace {

const auto kTau = 2 * M_PI;

}

int main(int argc, char* argv[]) {
    ef::Options options;
    if (!options.Parse(argc, argv)) {
        return 1;
    }

    options.Report();

    auto client = ColorClient::Create(options.color_addr());

    pkg::Theme theme;
    auto status = client->GetRandomTheme(
        &theme,
        options.seed());
    if (!status.ok()) {
        std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }

    std::default_random_engine rng;
    rng.seed(options.seed());

    std::vector<Color> colors;
    Color::GetAllFromTheme(&colors, theme);
    ef::SortColors(rng, colors);

    double width = 1600.0;
    double height = 600.0;

    std::uniform_int_distribution<int> ndist(50, 150);
    std::vector<std::shared_ptr<Charge>> charges;
    ef::CreateCharges(
        &charges,
        rng,
        width,
        height,
        ndist(rng));

    auto surface = Cairo::ImageSurface::create(
        Cairo::ImageSurface::Format::ARGB32,
        width,
        height);
    auto context = Cairo::Context::create(surface);

    colors[0].Set(*context);
    context->rectangle(0, 0, width, height);
    context->fill();

    context->save();
    std::vector<Vec2> path;
    path.reserve(1000);
    ef::FieldLineOptions opts(4.0);
    context->set_line_width(1.0);
    colors[4].SetWithAlpha(*context, 0.5);
    for (auto charge : charges) {
        auto da = kTau / 64;
        for (auto a = 0.0; a < kTau; a += da) {
            Vec2 off(opts.near() * cos(a), opts.near() * sin(a));
            ef::ComputeFieldLine(
                &path,
                opts,
                *charge,
                Vec2::add(charge->point(), off),
                charges);
            context->begin_new_path();
            auto it = path.begin();
            context->move_to(it->i(), it->j());
            it++;
            for (; it != path.end(); it++) {
                context->line_to(it->i(), it->j());
            }
            context->stroke();
        }
    }
    context->restore();

    context->save();
    context->set_line_width(2.0);
    for (auto charge : charges) {
        auto pt = charge->point();
        context->begin_new_path();
        context->arc(pt.i(), pt.j(), 3, 0, 2 * M_PI);
        colors[0].Set(*context);
        context->fill();
        colors[4].SetWithAlpha(*context, 0.5);
        context->stroke();
    }
    context->restore();

    surface->write_to_png(options.dst().c_str());

    return 0;
}