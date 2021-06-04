#include "color.h"

#include "pkg/theme.pb.h"

#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

double Color::R() const {
    double r = (rgb_ >> 16) & 0xff;
    return r / 255.0;
}

double Color::G() const {
    double g = (rgb_ >> 8) & 0xff;
    return g / 255.0;
}

double Color::B() const {
    double b = rgb_ & 0xff;
    return b / 255.0;
}

double Color::Luminance() const {
    return 0.2126 * R() + 0.75152 * G() + 0.0722 * B();
}

std::string Color::Hex() const {
    std::stringbuf buf;
    std::ostream os(&buf);
    os << std::hex << std::setw(6) << std::setfill('0') << rgb_;
    return buf.str();
}

void Color::Set(Cairo::Context& context) const {
    context.set_source_rgb(R(), G(), B());
}

void Color::SetWithAlpha(Cairo::Context& context, double alpha) const {
    context.set_source_rgba(R(), G(), B(), alpha);
}

void Color::GetAllFromTheme(
    std::vector<Color>* colors,
    const pkg::Theme& theme
) {
    colors->clear();
    colors->reserve(theme.swatches_size());
    for (auto c : theme.swatches()) {
        colors->push_back(Color(c));
    }
}