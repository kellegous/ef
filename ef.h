#ifndef _EF_H_
#define _EF_H_

#include <google/protobuf/stubs/common.h>
#include <memory>
#include <random>
#include <vector>

class Vec2;
class Color;
class Charge;

namespace ef {

void CreateCharges(
    std::vector<std::shared_ptr<Charge>>* dst,
    std::default_random_engine& rng,
    double w,
    double h,
    size_t n);


class FieldLineOptions {
public:
    FieldLineOptions(double near)
        : near_(near) {}

    GOOGLE_DISALLOW_IMPLICIT_CONSTRUCTORS(FieldLineOptions);

    double near() const {
        return near_;
    }
private:
    double near_;
};

void ComputeFieldLine(
    std::vector<Vec2>* path,
    const FieldLineOptions& opts,
    const Charge& charge,
    const Vec2& start,
    const std::vector<std::shared_ptr<Charge>>& charges);

void SortColors(
    std::default_random_engine& rng,
    std::vector<Color>& colors);

class Options {
public:
    Options(uint64_t seed,
        const std::string& color_addr,
        const std::string& dst)
        : seed_(seed),
          color_addr_(color_addr),
          dst_(dst) {}

    Options();

    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Options);

    uint64_t seed() const {
        return seed_;
    }

    std::string color_addr() const {
        return color_addr_;
    }

    std::string dst() {
        return dst_;
    }

    bool Parse(int argc, char* argv[]);
    void Report();
private:
    uint64_t seed_;
    std::string color_addr_;
    std::string dst_;
};

} // namespace ef

#endif // _EF_H_