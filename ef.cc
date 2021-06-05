#include "ef.h"

#include "charge.h"
#include "color.h"
#include "seed.h"
#include "vec2.h"

#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <libgen.h>

namespace {

bool IsNearCharge(
    const ef::FieldLineOptions& opts,
    const Vec2& pt,
    const std::vector<std::shared_ptr<Charge>>& charges) {
    for (auto charge : charges) {
        if (Vec2::sub(charge->point(), pt).length() < opts.near()) {
            return true;
        }
    }
    return false;
}

std::string GetDefaultDst(char* cmd) {
    std::string dst(basename(cmd));
    dst.append(".png");
    return dst;
}

}

namespace ef {

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
    for (auto i = 0; i < n; i++) {
        std::shared_ptr<Charge> c(
            new Charge(i*dx + dx/2, hdist(rng), cdist(rng))
        );
        dst->push_back(c);
    }
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
    std::vector<Color>& colors) {
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

Options::Options()
    : seed_(seed::FromTime()),
      color_addr_("192.168.7.23:8081") {
}

bool Options::Parse(int argc, char* argv[]) {
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

    if (dst_.empty()) {
        dst_ = GetDefaultDst(argv[0]);
    }

    return true;
}

void Options::Report() {
    std::cout << "[seed = " << std::hex << seed_ << std::dec
        << ", color_addr = " << color_addr_ << "]"
        << std::endl;
}

} // namespace ef