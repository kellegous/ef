#include "clr.h"
#include "color.h"
#include "color_client.h"
#include "seed.h"
#include "pkg/theme.pb.h"

#include <getopt.h>
#include <libgen.h>

namespace {

class Options {
public:
    Options(
        uint64_t seed,
        const std::string& color_addr)
        : seed_(seed),
          color_addr_(color_addr) {}

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
                if (!seed::Parse(&seed_, optarg)) {
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

    void Report() {
        std::cout << "[seed = " << std::hex << seed_
            << ", color_addr = " << color_addr_ << "]"
            << std::endl;
    }
private:
    uint64_t seed_;
    std::string color_addr_;
};

}

int main(int argc, char* argv[]) {
    Options options(seed::FromTime(), "192.168.7.23:8081");
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

    std::vector<Color> colors;
    Color::GetAllFromTheme(&colors, theme);

    for (auto color : colors) {
        std::cout << color.Hex() << std::endl;
    }

    double width = 1600.0;
    double height = 600;

    auto surface = Cairo::ImageSurface::create(
        Cairo::ImageSurface::Format::ARGB32,
        width,
        height);
    auto context = Cairo::Context::create(surface);

    auto dw = width / colors.size();

    for (std::size_t i = 0, n = colors.size(); i < n; i++) {
        auto x = i * dw;
        auto color = colors[i];
        color.Set(*context);
        context->rectangle(x, 0, dw, height);
        context->fill();

        context->save();
        if (color.Luminance() > 0.5) {
            context->set_source_rgb(0, 0, 0);
        } else {
            context->set_source_rgb(1, 1, 1);
        }

        std::string hex("#");
        hex.append(color.Hex());

        context->set_font_size(24);
        context->translate(x + 40, 40);
        context->show_text(hex);
        context->fill();
        context->restore();
    }

    std::string dst(basename(argv[0]));
    dst.append(".png");
    surface->write_to_png(dst.c_str());

    return 0;
}