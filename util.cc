#include "util.h"
#include <random>
#include <ctime>
namespace util {
   double random_real() {
    static std::minstd_rand0 generator(std::time(0));
    static std::uniform_real_distribution<double> dst(0, 1);
    return dst(generator);
  }
} // namespace.
