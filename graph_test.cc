#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>
#include <chrono>
#include <thread>
#include <future>
#include "three-spanner-algorithm.h"
#include "2k_spanner.h"
#include "json.hpp"

using namespace std;
using namespace graphs;
using json = nlohmann::json;


void log_last_graph(const Graph& g, const string& prefix = "") {
  ofstream last_graph_log(prefix + "last_graph_log.txt");
  last_graph_log << g;
}

template<typename SpannerAlg>
bool CreateSpannerEdgeNumberReport(SpannerAlg&& alg,
    int start_size,
    int end_size,
    int how_many_runs,
    const string& report_prefix = "",
    int inc = 1) {
  ofstream outfile(report_prefix + "graph_report.json");
  vector<json> size_ratio_vector;
  for (int size = start_size ; size < end_size; size += inc) {
    json run_for_size;
    run_for_size["size"] = size;
    long long running_spanner_edge_size = 0L;
    double running_average = 0.0;
    for(int j = 0; j < how_many_runs; ++j) {
      auto g = randomGraph(size);
      auto spanner = alg(g);
      auto part_of_average = double(g.edges()) / double(spanner.edges());
      assert(g.edges() >= spanner.edges());
      running_average += part_of_average;
      running_spanner_edge_size += spanner.edges();
    }
    run_for_size["average_ratio"] = running_average / double(how_many_runs);
    run_for_size["average_spanner_size"] = running_spanner_edge_size /
      how_many_runs;
    size_ratio_vector.emplace_back(std::move(run_for_size));
  }
  outfile << json(std::move(size_ratio_vector));
  return true;
}

// Represents the arguments needed for running an one of the spanner algorithms
// on random graphs.
struct ExperimentArgs {
  int graph_size;
  double graph_density;
  int k;  // Needed only for the 2k-1 spanner algorithm.
  int num_runs;  // How many random graphs to try it on.
  std::function<double(void)> edge_weight = util::random_real;
  ExperimentArgs(int size, const json& experiment_info):
    graph_size(size), graph_density(experiment_info["density"]),
        k(experiment_info.count("k") != 0 ? int(experiment_info["k"]) : -1),
        num_runs(experiment_info["num_runs_per_size"]) {}
  ExperimentArgs(int sz, double d, int k, int num_runs):
    graph_size(sz), graph_density(d), k(k), num_runs(num_runs) {}
};


template<typename SpannerAlg>
json EdgeNumberExperiment(SpannerAlg&& alg, const ExperimentArgs& args ) {
  json result;
  result["size"] = args.graph_size;
  result["k"] = args.k;
  result["density"] = args.graph_density;
  result["num_runs"] = args.num_runs;
  long long running_spanner_edge_size = 0L;
  for (int i = 0; i < args.num_runs; ++i) {
    auto g = randomGraph(args.graph_size, args.graph_density,
        args.edge_weight);
    auto spanner = alg(g);
    assert(g.edges() >= spanner.edges());
    running_spanner_edge_size += spanner.edges();
  }
  result["average_spanner_size"] = running_spanner_edge_size / args.num_runs;
  return result;
}

template<typename SpannerAlg>
void CreateSpannerStretchReport(SpannerAlg&& alg,
    int start_size,
    int end_size,
    int how_many_runs,
    const string& report_prefix = "") {
  auto create_strech_json = [&] (int graph_size) -> json {
    json res;
    res["size"] = graph_size;
    double running_stretch = 0.0;
    auto skip_strech_pair = [&] (auto&& g_dst, auto&& s_dst) -> bool {
      return g_dst == std::numeric_limits<double>::infinity() ||
        g_dst == 0;
    };

    for (int run = 0; run < how_many_runs; ++run) {
      auto g = randomGraph(graph_size);
      auto spanner = alg(g);
      auto dsts_g = floydwarshall(g);
      auto dsts_s = floydwarshall(spanner);
      int stretches_considered = 0;
      double max_stretch_for_g = 0.0;
      for (int i = 0; i < g.size(); ++i) {
        for(int j = i + 1; j < g.size(); ++j) {
          if (skip_strech_pair(dsts_g[i][j], dsts_s[i][j]))
            continue;
          // Stretch factor for this pair:
         if (dsts_s[i][j] == std::numeric_limits<double>::infinity()) {
           assert(false);
         }
          auto stretch = dsts_s[i][j] / dsts_g[i][j];
          assert(dsts_s[i][j] >= dsts_g[i][j]);
          max_stretch_for_g = std::max(max_stretch_for_g, stretch);
          ++stretches_considered;
        }
      }
      running_stretch += max_stretch_for_g;
    }
    res["average_stretch"] = running_stretch / double(how_many_runs);
    return res;
  };
  ofstream outfile(report_prefix + "StretchReport.json");
  outfile << "[" << std::endl;
  outfile << std::setw(4) << create_strech_json(start_size);
  for (int size = start_size + 1; size < end_size; ++size) {
    outfile << "," << std::endl << std::setw(4) << create_strech_json(size);
  }
  outfile << "]" << std::endl;
}


// Returns the maximum stretch for g, s where s is a subgraph of g.
auto MaxStretch(const Graph& g, const Graph& s) {
  auto dsts_g = floydwarshall(g);
  auto dsts_s = floydwarshall(s);
  double max_stretch = 0.0;
  // If two vertices were disconnected in g, we don't check for them, since
  // in such a case our algorithm would divide by \inf.
  auto skip_strech_pair = [&] (auto&& g_dst, auto&& s_dst) -> bool {
    return g_dst == std::numeric_limits<double>::infinity() ||
      g_dst == 0;
  };
  for (int i = 0; i < g.size(); ++i) {
    for(int j = i + 1; j < g.size(); ++j) {
      if (skip_strech_pair(dsts_g[i][j], dsts_s[i][j]))
        continue;
      // Stretch factor for this pair:
      if (dsts_s[i][j] == std::numeric_limits<double>::infinity()) {
        assert(false);
      }
      auto stretch = dsts_s[i][j] / dsts_g[i][j];
      assert(dsts_s[i][j] >= dsts_g[i][j]);
      max_stretch = std::max(max_stretch, stretch);
    }
  }
  return max_stretch;
}

template<typename SpannerAlg>
json MaxStretchExperiment(SpannerAlg&& alg, const ExperimentArgs& args ) {
  json result;
  result["size"] = args.graph_size;
  result["k"] = args.k;
  result["density"] = args.graph_density;
  result["num_runs"] = args.num_runs;
  double max_stretch = 0.0;
  for (int i = 0; i < args.num_runs; ++i) {
    auto g = randomGraph(args.graph_size, args.graph_density,
        args.edge_weight);
    auto spanner = alg(g);
    max_stretch = std::max(max_stretch, MaxStretch(g, spanner));
  }
  result["max_stretch"] = max_stretch;
  return result;
}


constexpr char kConfigFileFlag[] = "experiments_config_file";

void init(int argc, char** argv) {
  util::add_string_flag(kConfigFileFlag,
      "Configuration file for running experiments, see README for more info",
      "in/config.json");
  util::add_string_flag("report_suffix",
      "Suffix to be appended to all reports generated by this run, "
      "Defaults to a random 6 character string",
      util::random_string(7));
  util::add_bool_flag("use_new_alg", "use the rewrite of baswana 2k-1 or not",
      true);
  util::parse_flags(argc, argv);
}

enum class  AlgorithmType {
  THREE_SPANNER,
  TWO_K_SPANNER,
  TWO_K_SPANNER2,
};

enum class ExperimentType {
  EDGE_COUNT,
  MAX_STRETCH,
  DENSITY,
};

ExperimentType TypeFromString(const string& type) {
  constexpr char kEdgeCount[] = "EdgeCount";
  constexpr char kMaxStretch[] = "MaxStretch";
  constexpr char kDensity[] = "Density";
  if (type == kEdgeCount)
    return ExperimentType::EDGE_COUNT;
  if (type == kMaxStretch)
    return ExperimentType::MAX_STRETCH;
  if (type == kDensity)
    return ExperimentType::DENSITY;
  std::cout << "Invalid experiment typename must be " << kEdgeCount << " or "
    << kMaxStretch;
  assert(false);
}

class Experimentor {
  public:
    json operator()(const ExperimentArgs& args) {
      return func(args);
    };
    
    // Creates an Experimentor object, that once called will conduct on the
    // correct experiment according to 'alg_type' and 'exp_type'.
    static Experimentor CreateExperimentor(AlgorithmType alg_type,
        ExperimentType exp_type) {
      static auto three_span = [] (auto&& g) {return three_spanner(g);};
      switch (alg_type) {
        case AlgorithmType::THREE_SPANNER:
          switch (exp_type) {
            case ExperimentType::EDGE_COUNT: 
             return {[] (const ExperimentArgs& args) {
               return EdgeNumberExperiment(three_spanner, args);
             }};
            case ExperimentType::MAX_STRETCH:
             return { [] (const ExperimentArgs& args) -> json {
               return MaxStretchExperiment(three_spanner, args);
             }};
            default:
             cout << "unimplemented " << endl;
             assert(false);
          }

        case AlgorithmType::TWO_K_SPANNER:
          switch (exp_type) {
            case ExperimentType::EDGE_COUNT: 
             return {[] (const ExperimentArgs& args) {
               return EdgeNumberExperiment([k=args.k] (auto&& g) {
                   return two_k_minus_1_spanner(k, g);}, args);
             }};
            case ExperimentType::MAX_STRETCH:
             return {[] (const ExperimentArgs& args) {
               return MaxStretchExperiment([k=args.k] (auto&& g) {
                   return two_k_minus_1_spanner(k, g);}, args);
             }};
            case ExperimentType::DENSITY:
             return {[] (const ExperimentArgs& args) {
               return MaxStretchExperiment([k=args.k] (auto&& g) {
                   return two_k_minus_1_spanner(k, g);}, args);
             }};
          }
        case AlgorithmType::TWO_K_SPANNER2:
          switch (exp_type) {
            case ExperimentType::EDGE_COUNT: 
             return {[] (const ExperimentArgs& args) {
               return EdgeNumberExperiment([k=args.k] (auto&& g) {
                   return two_k_minus_1_spannerv2(k, g);}, args);
             }};
            case ExperimentType::MAX_STRETCH:
             return {[] (const ExperimentArgs& args) {
               return MaxStretchExperiment([k=args.k] (auto&& g) {
                   return two_k_minus_1_spannerv2(k, g);}, args);
             }};
            case ExperimentType::DENSITY:
             return {[] (const ExperimentArgs& args) {
               return EdgeNumberExperiment([k=args.k] (auto&& g) {
                   return two_k_minus_1_spannerv2(k, g);}, args);
             }};
          }
      } 
    }
  private:
    using f_type = std::function<json(const ExperimentArgs& args)>;
    Experimentor(f_type f): func(f) {}
    f_type func;
};


class ExperimentInfos {
  public:
    int size() {return args.size();}
    auto begin() { return args.begin();}
    auto end() { return args.end();}
    ExperimentInfos(ExperimentType type, const json& exp_info) {
      auto weight_dist = edge_weight_from_exp(exp_info);
      switch (type) {
        case ExperimentType::DENSITY:
          for (auto&& density : exp_info["densities"]) {
            args.emplace_back(exp_info["size"], density, exp_info["k"],
                exp_info["num_runs_per_size"]);
            args.back().edge_weight = edge_weight_from_exp(exp_info); 
          }
          break;
        case ExperimentType::EDGE_COUNT:
        case ExperimentType::MAX_STRETCH:
          for (auto&& size : exp_info["sizes"]) {
            args.emplace_back(size, exp_info);
            args.back().edge_weight = edge_weight_from_exp(exp_info); 
          }
          break;
      }
    }

  private:
  std::vector<ExperimentArgs> args; 
  std::function<double(void)> edge_weight_from_exp(const json& exp_info) {
    if (exp_info.count("edge_weight_distribution") == 0)
      return util::random_real;
    const auto& type_name = exp_info["edge_weight_distribution"];
    if (type_name == "exponential") {
      double mean = exp_info.count("mean") ? double(exp_info["mean"]) : 0.05;
      return [mean] () -> double {
        static std::minstd_rand0 generator(std::time(0));
        static std::exponential_distribution<double> dst(mean);
        return dst(generator);
      };
    }
    if (type_name == "gamma") {
      return [] () -> double {
        static std::minstd_rand0 generator(std::time(0));
        static std::gamma_distribution<double> dst;
        return dst(generator);
      };
    }
    if (type_name == "weibull") {
      return [] () -> double {
        static std::minstd_rand0 generator(std::time(0));
        static std::weibull_distribution<double> dst;
        return dst(generator);
      };
    }
    cout << "Unrecognized edge_weight_distribution " << type_name << endl;
    assert(false);
  }
};

string output_file(const string& filename) {
  const string prefix = "/Users/bilalSaad/ws/t-span/build/out/";
  std::time_t t = std::time(nullptr);
  char mbstr[10];
  string dir;
  if(std::strftime(mbstr, sizeof(mbstr), "%h%d/", std::localtime(&t))) {
    dir = mbstr;
    string command = "mkdir -p " + prefix + mbstr;
    std::system(command.c_str());
  }
  return prefix + dir + filename;
}


void ConductExperiments(const json& experiment_config,
    const string& report_suffix, AlgorithmType alg_type) {
  auto experiment_conductor = [&report_suffix] (auto&& exp_func,
      int experiment_index, auto&& exp_info, auto&& type) {
    ExperimentInfos experiments(type, exp_info);
    std::vector<std::future<json>> futures; 
    for (auto&& experiment_args : experiments) {
      futures.emplace_back(std::async(exp_func, experiment_args));
    }
    auto filename = "Report" + std::to_string(experiment_index) + "_" +
      report_suffix +".json";
    ofstream outfile( output_file(filename));
    outfile << "[";
    string separator = "";
    for (auto& f : futures) {
      auto res = f.get();
      outfile << separator << std::endl << std::setw(4) << res;
      separator = ",";
    }
    outfile << "]";
  };

  std::vector<std::future<void>> futures(experiment_config.size()); 
  for (int i = 0; i < experiment_config.size(); ++i) {
    auto exp_type = TypeFromString(experiment_config[i]["type"]);
    auto exp_runner = Experimentor::CreateExperimentor(alg_type, exp_type);
    futures[i] = std::async(experiment_conductor, exp_runner, i,
        experiment_config[i], exp_type);
  }
  for (auto& f : futures) {
    f.get();
  }
}

auto load_configuration(const string& fname) {
  ifstream config_file(fname);
  json config_json;
  config_file >> config_json;
  return config_json;
}

auto AlgorithmTypeFromString(const string& alg) {
  if (alg == "2k_spanner") {
    return AlgorithmType::TWO_K_SPANNER;
  } else if (alg == "3_spanner") {
    return AlgorithmType::THREE_SPANNER;
  } else if (alg == "2k_spanner2") {
    return AlgorithmType::TWO_K_SPANNER2;
  } else {
    std::cout << "Invalid typename must be 2k_spanner{2,} or 3_spanner.";
    assert(false);
  }
}

int main(int argc, char** argv) {
  init(argc, argv);
  auto config_json = load_configuration(util::get_string_flag(kConfigFileFlag));
  const auto& experiments = config_json["experiments"];
  auto type = AlgorithmTypeFromString(config_json["type"]);
  const std::string& report_suffix = util::get_string_flag("report_suffix"); 
  cout << report_suffix << endl;
  ConductExperiments(experiments, report_suffix, type);
  return 0;
}
