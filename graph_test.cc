// TODO this file
#include "three-spanner-algorithm.h"
#include "2k_spanner.h"
#include "json.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>
#include <chrono>
#include <thread>
#include <future>
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

template<typename SpannerAlg>
json EdgeNumberExperiment(SpannerAlg&& alg, int size, double density, int k,
    int num_runs) {
  json result;
  result["size"] = size;
  result["k"] = k;
  result["density"] = density;
  result["num_runs"] = num_runs;
  long long running_spanner_edge_size = 0L;
  double running_average = 0.0;
  for (int i = 0; i < num_runs; ++i) {
    auto g = randomGraph(size, density);
    auto spanner = alg(g);
    auto part_of_average = double(g.edges()) / double(spanner.edges());
    assert(g.edges() >= spanner.edges());
    running_average += part_of_average;
    running_spanner_edge_size += spanner.edges();
  }
  result["average_ratio"] = running_average / double(num_runs);
  result["average_spanner_size"] = running_spanner_edge_size / num_runs;
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

constexpr char kConfigFileFlag[] = "experiments_config_file";

void init(int argc, char** argv) {
  util::add_string_flag(kConfigFileFlag,
      "Configuration file for running experiments, see README for more info",
      "");
  util::parse_flags(argc, argv);
  assert(util::is_flag_set("experiments_config_file"));
}

void ConductExperiments(const json& experiment_config) {
  auto edge_count_experiment = [] (int experiment_index, auto&& exp_info) {
    std::vector<std::future<json>> futures(exp_info["sizes"].size()); 
    for (int i = 0; i < futures.size(); ++i) {
      auto&& sz = exp_info["sizes"][i];
      int k = exp_info["k"];
      futures[i] = std::async([&] () {
            return EdgeNumberExperiment([k] (auto&& g) {
                return two_k_minus_1_spanner(k, g);
                }, sz, exp_info["density"], k, exp_info["num_runs_per_size"]);
          });
    }
    ofstream outfile(
        "out/EdgeReport " + std::to_string(experiment_index) + " _ "
        + util::random_string(7) + ".json"); 
    outfile << "[";
    string separator = "";
    for (auto& f : futures) {
      auto res = f.get();
      outfile << separator << std::endl << std::setw(4) << res;
      separator = ",";
    }
    outfile << "]";
  };


  std::cout << experiment_config << std::endl;
  constexpr char kEdgeCount[] = "EdgeCount";
  std::vector<std::future<void>> futures(experiment_config.size()); 
  for (int i = 0; i < experiment_config.size(); ++i) {
    if (experiment_config[i]["type"] == kEdgeCount) {
      futures[i] = std::async(edge_count_experiment, i, experiment_config[i])  ;
    }
  }
  for (auto& f : futures) {
    f.get();
  }

}

int main(int argc, char** argv) {
  init(argc, argv);
  ifstream config_file(util::get_string_flag(kConfigFileFlag));
  json config_json;
  config_file >> config_json;
  ConductExperiments(config_json);
  return 0;
}
