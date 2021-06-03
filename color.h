#ifndef _COLOR_H_
#define _COLOR_H_

#include <cairomm/cairomm.h>
#include <cinttypes>

class Color {
public:
    Color(uint32_t rgb): rgb_(rgb) {}
    Color(const Color& c): rgb_(c.rgb_) {}

    double R() const;
    double G() const;
    double B() const;

    double Luminance() const;
    std::string Hex() const;
    
    void Set(Cairo::Context& context) const;
    void SetWithAlpha(
        Cairo::Context& context,
        double alpha) const;
private:
    uint32_t rgb_;
};

#endif // _COLOR_H_