#ifndef _VEC2_H_
#define _VEC2_H_

class Vec2 {
private:
    double _i;
    double _j;
public:
    Vec2(double i, double j) noexcept: _i(i), _j(j) {}

    Vec2(Vec2& v) noexcept: _i(v._i), _j(v._j) {}

    Vec2(const Vec2& v) noexcept: _i(v._i), _j(v._j) {}

    Vec2(Vec2&& v) noexcept: _i(v._i), _j(v._j) {}

    Vec2& operator=(const Vec2& v) {
        _i = v.i();
        _j = v.j();
        return *this;
    }

    double i() const {
        return _i;
    }

    double j() const {
        return _j;
    }

    static Vec2 origin() {
        return Vec2(0, 0);
    }

    double length() const;

    Vec2 scale(double f);

    static Vec2 add(
        const Vec2& a,
        const Vec2& b);

    static Vec2 sub(
        const Vec2& a,
        const Vec2& b);

    static double dot(
        const Vec2& a,
        const Vec2& b);

    static double angleBetween(
        const Vec2& a,
        const Vec2& b);
};

#endif