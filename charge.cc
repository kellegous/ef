#include "charge.h"

#include <sstream>

Charge::Charge(
    double i,
    double j,
    double c): _point(i, j), _c(c) {
}

Charge::Charge(
    const Vec2& point,
    double c): _point(point), _c(c) {
}

Vec2 Charge::point() {
    return _point;
}

double Charge::c() {
    return _c;
}

std::string Charge::description() {
    std::ostringstream ss;
    ss << "i = " << _point.i() << ", ";
    ss << "j = " << _point.j() << ", ";
    ss << "c = " << _c;
    return ss.str();
}