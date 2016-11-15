#ifndef UTIL_H
#define UTIL_H
#include <chrono>
#include <string>
#include <iostream>

namespace util {
  using Clock = std::chrono::high_resolution_clock;
  using std::chrono::time_point;
  using std::chrono::duration_cast;
  using timeunit = std::chrono::duration<double>;
  using namespace std::literals::chrono_literals;
  class scoped_timer {
    public:
      scoped_timer(const std::string& s = ""): start_time(Clock::now()),
        name(s) {}
      ~scoped_timer() {
        auto end_time = Clock::now();
        auto diff = duration_cast<timeunit>(end_time - start_time);
        std::cout << "Time elapsed " << (name.empty() ? "" : "for " + name)
          << ": " << diff.count() << " seconds" << std::endl;
      }
    private:
      time_point<Clock> start_time; 
      std::string name;
  };
} // namespace util.
#endif
