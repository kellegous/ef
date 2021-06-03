#include "charge.h"
#include "pkg/rpc/rpc.grpc.pb.h"

#include <cairomm/cairomm.h>
#include <chrono>
#include <getopt.h>
#include <grpcpp/grpcpp.h>
#include <iomanip>
#include <iostream>
#include <libgen.h>
#include <memory>
#include <random>
#include <sstream>
#include <vector>

namespace {

const auto kTau = 2 * M_PI;

void Seed(std::default_random_engine& rng) {
    auto t = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    );
    rng.seed(t.count());
}

uint64_t SeedFromTime() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

bool ParseSeed(uint64_t* v, const std::string& s) {
    std::istringstream ss(s);
    ss >> std::hex >> *v;
    return !ss.fail();
}

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
        const std::string& color_addr)
        : seed_(seed), color_addr_(color_addr) {}

    Options() : seed_(0) {}

    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Options);

    uint64_t seed() const {
        return seed_;
    }

    std::string color_addr() const {
        return color_addr_;
    }

    bool Parse(int argc, char* argv[]) {
        int c;
        struct option long_options[] = {
            {"seed", required_argument, 0, 's'},
            {"color_addr", required_argument, 0, 'c'},
            {0, 0, 0, 0},
        };

        while (true) {
            auto option_index = 0;

            c = getopt_long(argc, argv, "s:c:", long_options, &option_index);
            if (c == -1) {
                break;
            }

            switch (c) {
            case 's':
                if (!ParseSeed(&seed_, optarg)) {
                    return false;
                }
                break;
            case 'c':
                color_addr_ = optarg;
                break;
            case '?':
                return false;
            }
        }

        return true;
    }
private:
    uint64_t seed_;

    std::string color_addr_;
};

class Color {
public:
    Color(uint32_t rgb): rgb_(rgb) {}
    Color(const Color& c): rgb_(c.rgb_) {}

    double R() {
        double r = (rgb_ >> 16) & 0xff;
        return r / 255.0;
    }

    double G() {
        double g = (rgb_ >> 8) & 0xff;
        return g / 255.0;
    }

    double B() {
        double b = rgb_ & 0xff;
        return b / 255.0;
    }

    std::string Hex() {
        std::stringbuf buf;
        std::ostream os(&buf);
        os << std::hex << std::setw(6) << std::setfill('0') << rgb_;
        return buf.str();
    }

    void Set(Cairo::Context& context) {
        context.set_source_rgb(R(), G(), B());
    }

    void SetWithAlpha(
        Cairo::Context& context,
        double alpha) {
        context.set_source_rgba(R(), G(), B(), alpha);
    }
private:
    uint32_t rgb_;
};

void GetColorsFromTheme(
    std::vector<Color>* colors,
    const pkg::Theme& theme) {
    colors->clear();
    colors->reserve(theme.swatches_size());
    for (auto c : theme.swatches()) {
        colors->push_back(Color(c));
    }
}

class ColorClient {
public:
    ColorClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(rpc::Colors::NewStub(channel)) {}

    grpc::Status GetRandomTheme(pkg::Theme* theme) {
        rpc::RandReq req;
        req.set_n(1);

        rpc::RandRes res;

        grpc::ClientContext context;

        grpc::Status status = stub_->GetRandom(&context, req, &res);
        if (!status.ok()) {
            return status;
        }

        theme->CopyFrom(res.themes().Get(0));
        return status;
    }

    static std::unique_ptr<ColorClient> Create(const std::string& addr) {
        return std::unique_ptr<ColorClient>(
            new ColorClient(grpc::CreateChannel(addr, grpc::InsecureChannelCredentials())));
    }
private:
    std::unique_ptr<rpc::Colors::Stub> stub_;
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

}

int main(int argc, char* argv[]) {
    Options options(SeedFromTime(), "192.168.7.23:8081");
    if (!options.Parse(argc, argv)) {
        return 1;
    }

    std::cout << "seed = " << options.seed()
        << " color_addr = " << options.color_addr()
        << std::endl;

    auto client = ColorClient::Create(options.color_addr());

    pkg::Theme theme;
    auto status = client->GetRandomTheme(&theme);
    if (!status.ok()) {
        std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }

    std::vector<Color> colors;
    GetColorsFromTheme(&colors, theme);
    for (auto color : colors) {
        std::cout << "#" << color.Hex() << std::endl;
    }

    double width = 1600.0;
    double height = 600.0;

    std::default_random_engine rng;
    Seed(rng);

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
    context->set_line_width(3.0);
    colors[3].SetWithAlpha(*context, 0.5);
    for (auto charge : charges) {
        auto da = kTau / 64;
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

    colors[1].Set(*context);
    for (auto charge : charges) {
        auto pt = charge->point();
        context->begin_new_path();
        context->arc(pt.i(), pt.j(), 2, 0, 2 * M_PI);
        context->fill();
    }

    std::string dst(basename(argv[0]));
    dst.append(".png");
    surface->write_to_png(dst.c_str());

    return 0;
}