#include "charge.h"
#include "pkg/rpc/rpc.grpc.pb.h"

#include <cairomm/cairomm.h>
#include <grpcpp/grpcpp.h>
#include <iomanip>
#include <iostream>
#include <libgen.h>
#include <memory>
#include <random>
#include <sstream>
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

}

int main(int argc, char* argv[]) {
    auto client = ColorClient::Create("192.168.7.23:8081");

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

    std::vector<std::shared_ptr<Charge>> charges;
    CreateCharges(&charges, width, height, 20);

    auto surface = Cairo::ImageSurface::create(
        Cairo::ImageSurface::Format::ARGB32,
        width,
        height);
    auto context = Cairo::Context::create(surface);

    colors[0].Set(*context);
    context->rectangle(0, 0, width, height);
    context->fill();

    colors[1].Set(*context);
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