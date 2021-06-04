#include "charge.h"
#include "color.h"
#include "color_client.h"
#include "seed.h"
#include "vec2.h"

#include <cairomm/cairomm.h>
#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <libgen.h>
#include <memory>
#include <random>
#include <vector>

namespace {

const auto kTau = 2 * M_PI;

void CreateCharges(
    std::vector<std::shared_ptr<Charge>>* dst,
    std::default_random_engine& rng,
    double w,
    double h,
    size_t n) {
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

class Options {
public:
    Options(
        uint64_t seed,
        const std::string& color_addr,
        const std::string& dst)
        : seed_(seed),
          color_addr_(color_addr),
          dst_(dst) {}

    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Options);

    uint64_t seed() const {
        return seed_;
    }

    std::string color_addr() const {
        return color_addr_;
    }

    std::string dst() {
        return dst_;
    }

    bool Parse(int argc, char* argv[]) {
        int c;
        struct option long_options[] = {
            {"seed", required_argument, 0, 's'},
            {"color_addr", required_argument, 0, 'c'},
            {"dst", required_argument, 0, 'd'},
            {0, 0, 0, 0},
        };

        while (true) {
            auto option_index = 0;

            c = getopt_long(argc, argv, "s:c:d:", long_options, &option_index);
            if (c == -1) {
                break;
            }

            switch (c) {
            case 's':
                if (!seed::Parse(&seed_, optarg)) {
                    return false;
                }
                break;
            case 'c':
                color_addr_ = optarg;
                break;
            case 'd':
                dst_ = optarg;
                break;
            case '?':
                return false;
            }
        }

        return true;
    }

    void Report() {
        std::cout << "[seed = " << std::hex << seed_
            << ", color_addr = " << color_addr_ << "]"
            << std::endl;
    }
private:
    uint64_t seed_;

    std::string color_addr_;

    std::string dst_;
};

class FieldLineOptions {
public:
    FieldLineOptions(double near)
        : near_(near) {}

    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(FieldLineOptions);
    
    double near() const {
        return near_;
    }
private:
    double near_;
};

bool IsNearCharge(
    const FieldLineOptions& opts,
    const Vec2& pt,
    const std::vector<std::shared_ptr<Charge>>& charges) {
    for (auto charge : charges) {
        if (Vec2::sub(charge->point(), pt).length() < opts.near()) {
            return true;
        }
    }
    return false;
}

void ComputeFieldLine(
    std::vector<Vec2>* path,
    const FieldLineOptions& opts,
    const Charge& charge,
    const Vec2& start,
    const std::vector<std::shared_ptr<Charge>>& charges) {
    path->clear();

    Vec2 curr(start);
    path->push_back(curr);
    for (auto k = 0; k < 1000; k++) {
        Vec2 dir = Vec2::origin();
        for (auto c : charges) {
            auto dist = Vec2::sub(curr, c->point());
            auto distSq = dist.i()*dist.i() + dist.j()*dist.j();
            auto force = (charge.c() < 0 ? -1 : 1) * c->c() / distSq;
            dir = Vec2::add(
                dir,
                Vec2(dist.i() * force, dist.j() * force)
            );
        }
        curr = Vec2::add(
            curr,
            dir.scale(7 / dir.length())
        );
        path->push_back(curr);

        if (IsNearCharge(opts, curr, charges)) {
            return;
        }
    }
}

void SortColors(
    std::default_random_engine& rng,
    std::vector<Color>& colors
) {
    std::bernoulli_distribution d(0.5);
    auto fn = d(rng)
        ? [](const Color& a, const Color& b) -> bool {
            return a.Luminance() > b.Luminance();
        }
        : [](const Color& a, const Color& b) -> bool {
            return a.Luminance() < b.Luminance();
        };

        std::sort(colors.begin(), colors.end(), fn);
}

std::string GetDefaultDest(char* cmd) {
    std::string dst(basename(cmd));
    dst.append(".png");
    return dst;
}

}

int main(int argc, char* argv[]) {
    Options options(
        seed::FromTime(),
        "192.168.7.23:8081",
        GetDefaultDest(argv[0]));
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
    SortColors(rng, colors);

    double width = 1600.0;
    double height = 600.0;

    std::vector<std::shared_ptr<Charge>> charges;
    CreateCharges(
        &charges,
        rng,
        width,
        height,
        20);

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
    FieldLineOptions opts(4.0);
    context->set_line_width(1.0);
    colors[4].SetWithAlpha(*context, 0.5);
    for (auto charge : charges) {
        auto da = kTau / 128;
        for (auto a = 0.0; a < kTau; a += da) {
            Vec2 off(opts.near() * cos(a), opts.near() * sin(a));
            ComputeFieldLine(
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