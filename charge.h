#ifndef _CHARGE_H_
#define _CHARGE_H_

#include "vec2.h"

#include <string>

class Charge {
public:
    Charge(double i, double j, double c) noexcept;
    Charge(const Vec2& point, double c) noexcept;

    Charge(const Charge&) = delete;

    Vec2 point() const;
    double c() const;
    std::string description() const;
private:
    Vec2 _point;
    double _c;
};

#endif