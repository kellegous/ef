#include "seed.h"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace seed {

uint64_t FromTime() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

bool Parse(uint64_t* v, const std::string& s) {
    std::istringstream ss(s);
    ss >> std::hex >> *v;
    return !ss.fail();
}

}