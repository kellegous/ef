#ifndef _COLOR_H_
#define _COLOR_H_

#include <cairomm/cairomm.h>
#include <cinttypes>

namespace pkg {
    class Theme;
}

class Color {
public:
    Color(uint32_t rgb) noexcept: rgb_(rgb) {}
    Color(const Color& c) noexcept: rgb_(c.rgb_) {}

    double R() const;
    double G() const;
    double B() const;

    double Luminance() const;
    std::string Hex() const;
    
    void Set(Cairo::Context& context) const;
    void SetWithAlpha(
        Cairo::Context& context,
        double alpha) const;

    static void GetAllFromTheme(
        std::vector<Color>* colors,
        const pkg::Theme& theme);
private:
    uint32_t rgb_;
};

#endif // _COLOR_H_