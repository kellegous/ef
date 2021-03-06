#include "efb.h"

#include "charge.h"
#include "color.h"
#include "color_client.h"
#include "ef.h"

#include <cairomm/cairomm.h>
#include <math.h>
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

    auto client = ColorClient::Create(
        options.color_addr());
    
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
    int n = ndist(rng);
    std::cout << "n = " << n << std::endl;
    ef::CreateCharges(
        &charges,
        rng,
        width,
        height,
        n);

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
    ef::FieldLineOptions opts(2.0);
    // colors[4].Set(*context);
    colors[4].SetWithAlpha(*context, 0.5);
    std::uniform_real_distribution<double> adist(0.0, kTau);
    for (auto charge : charges) {
        for (auto i = 0; i < 128; i++) {
            auto a = adist(rng);
            Vec2 off(opts.near() * cos(a), opts.near() * sin(a));
            ef::ComputeFieldLine(
                &path,
                opts,
                *charge,
                Vec2::add(charge->point(), off),
                charges);
            for (auto pt : path) {
                context->begin_new_path();
                context->arc(pt.i(), pt.j(), 1.0, 0, kTau);
                context->fill();
            }
        }
    }
    context->restore();

    surface->write_to_png(options.dst().c_str());

    return 0;
}