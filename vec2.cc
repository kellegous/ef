#include "vec2.h"

#include <cmath>

double Vec2::length() {
    return sqrt(_i*_i + _j*_j);
}

Vec2 Vec2::scale(double f) {
    return Vec2(f * _i, f * _j);
}

Vec2 Vec2::add(
    const Vec2& a,
    const Vec2& b
) {
    return Vec2(a._i + b._i, a._j + b._j);
}

Vec2 Vec2::sub(
    const Vec2& a,
    const Vec2& b
) {
    return Vec2(a._i - b._i, a._j - b._j);
}

double Vec2::dot(
    const Vec2& a,
    const Vec2& b
) {
    return a._i*b._i + a._j*b._j;
}

double Vec2::angleBetween(
    const Vec2& a,
    const Vec2& b
) {
    return atan2(b._j, b._i) - atan2(a._j, a._i);
}