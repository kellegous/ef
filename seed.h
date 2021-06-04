#ifndef _SEED_H_
#define _SEED_H_

#include <cinttypes>
#include <string>

namespace seed {

uint64_t FromTime();

bool Parse(uint64_t* v, const std::string& s);

}

#endif // _SEED_H_