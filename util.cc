
#include "util.h"
#include <random>
#include <ctime>
namespace util {
using std::string;
using std::experimental::optional;
double random_real(optional<int> seed) {
  static std::minstd_rand0 generator((seed ? *seed : std::time(0)));
  static std::uniform_real_distribution<double> dst(0, 1);
  return dst(generator);
}
namespace {
  template<typename T>
    struct Flag {
      string description;
      bool is_set;
      T val;
    };
  static std::unordered_map<string, Flag<int>> INT_FLAGS;
  static std::unordered_map<string, Flag<string>> STRING_FLAGS;
  static std::unordered_map<string, Flag<bool>> BOOL_FLAGS;
  static std::unordered_map<string, Flag<double>> DOUBLE_FLAGS;
}  // namespace
void add_int_flag(const string& fname, const string& desc, int v) {
  INT_FLAGS.emplace(fname, Flag<int>{desc, false, v});
}
void add_string_flag(const string& fname, const string& desc, string v) {
  STRING_FLAGS.emplace(fname, Flag<string>{desc, false, v});
}
void add_bool_flag(const string& fname, const string& desc, bool v) {
  BOOL_FLAGS.emplace(fname, Flag<bool>{desc, false, v});
}

void add_double_flag(const string& fname, const string& desc, double v) {
  DOUBLE_FLAGS.emplace(fname, Flag<double>{desc, false, v});
}

int get_int_flag(const string& fname) {
  return INT_FLAGS[fname].val;
}
string get_string_flag(const string& fname) {
  return STRING_FLAGS[fname].val;
}
bool get_bool_flag(const string& fname) {
  return BOOL_FLAGS[fname].val;
}

double get_double_flag(const string& fname) {
  return DOUBLE_FLAGS[fname].val;
}
namespace {
  std::pair<string, string> parse_flag_name(char* arg) {
    while(*arg == '-')
      ++arg;
    string name;
    while (*arg != '=' && *arg != 0) {
      name.push_back(*(arg++));
    }
    assert(*arg != 0);
    ++arg;
    string val;
    while(*arg != 0) {
      val.push_back(*(arg++));
    }
    return {name, val};
  }


  enum class FLAG_TYPE {
    INT,
    BOOL,
    STRING,
    DOUBLE,
    UNKNOWN,
  };
  FLAG_TYPE get_flag_type(const string& fname) {
    if (INT_FLAGS.count(fname) != 0) {  // int flag
      return FLAG_TYPE::INT;
    } else if (STRING_FLAGS.count(fname) != 0) {  // string flag
      return FLAG_TYPE::STRING;
    } else if (BOOL_FLAGS.count(fname) != 0) {  // bool flag
      return FLAG_TYPE::BOOL;
    } else if (DOUBLE_FLAGS.count(fname) != 0) {
      return FLAG_TYPE::DOUBLE;
    } else {  // unrecognized flag
      return FLAG_TYPE::UNKNOWN;
    }
  }
}  // namespace

void parse_flags(int argc, char** argv) {
  // first argv is the program name.
  ++argv;
  for (int i = 1; i < argc; ++i) {
    auto name_and_val = parse_flag_name(*argv);
    const auto& fname = name_and_val.first;
    const auto& fval = name_and_val.second;
    auto flag_type = get_flag_type(fname);
    switch (flag_type) {
      case FLAG_TYPE::INT:
        INT_FLAGS[fname].val = std::stoi(fval); 
        INT_FLAGS[fname].is_set = true;
        break;
      case FLAG_TYPE::BOOL:
        BOOL_FLAGS[fname].val = fval == "true";
        BOOL_FLAGS[fname].is_set = true;
        break;
      case FLAG_TYPE::STRING: 
        STRING_FLAGS[fname].val = fval;
        STRING_FLAGS[fname].is_set = true;
        break;
      case FLAG_TYPE::DOUBLE:
        DOUBLE_FLAGS[fname].val = std::stod(fval);
        DOUBLE_FLAGS[fname].is_set = true;
        break;
      case FLAG_TYPE::UNKNOWN:
        std::cout << "Unrecognized flag " << fname << std::endl;
        assert(false);
    }
    ++argv;
  }
}

namespace {
  template<typename Visitor>
  bool visit_flag(const string& fname, Visitor&& visitor) {
    auto flag_type = get_flag_type(fname);
    switch (flag_type) {
      case FLAG_TYPE::INT:
        visitor(INT_FLAGS[fname]);
        break;
      case FLAG_TYPE::BOOL:
        visitor(BOOL_FLAGS[fname]);
        break;
      case FLAG_TYPE::STRING: 
        visitor(STRING_FLAGS[fname]);
        break;
      case FLAG_TYPE::DOUBLE:
        visitor(DOUBLE_FLAGS[fname]);
        break;
      case FLAG_TYPE::UNKNOWN:
        std::cout << "Unrecognized flag " << fname << std::endl;
        return false;
    }
    return true;
  }
}  // namespace

bool is_flag_set(const string& fname) {
  bool is_set = false;
  visit_flag(fname, [&is_set] (auto&& flag) {
      is_set = flag.is_set;
      });
  return is_set;
}

std::string random_string( size_t length ) {
    auto randchar = []() -> char {
        constexpr char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        srand(time(0));
        long random_number = random_real() * 10003;
        return charset[ random_number % max_index ];
    };
    std::string str(length,0);
    std::generate_n(str.begin(), length, randchar );
    return str;
}

} // namespace.
