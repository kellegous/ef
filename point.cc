#include "point.h"

#include <cmath>

double Point::distance(
    const Point& a,
    const Point& b
) {
    double dx = b._x - a._x;
    double dy = b._y - a._y;
    return sqrt(dx*dx + dy*dy);
}