#ifndef _CHARGE_H_
#define _CHARGE_H_

#include "vec2.h"

#include <string>

class Charge {
public:
    Charge(double i, double j, double c);
    Charge(const Vec2& point, double c);

    // Disallow copy
    Charge(const Charge&);

    Vec2 point();
    double c();
    std::string description();
private:
    Vec2 _point;
    double _c;
};

#endif