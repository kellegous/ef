#ifndef _POINT_H_
#define _POINT_H_

class Point {
private:
    double _x;
    double _y;
public:
    Point(double x, double y): _x(x), _y(y) {
    }

    double x() {
        return _x;
    }

    double y() {
        return _y;
    }

    static double distance(
        const Point& a,
        const Point& b);
};

#endif