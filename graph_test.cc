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
using namespace std;
using namespace graphs;
using json = nlohmann::json;


Graph evil_graph() {
  Graph g(6);
  // edges for 0
  vector<Edge> edges = {{4,12}, {3,84}, {1,57}};
  g.add_vertex_with_edges(0, edges);
  // edges for 1
  edges = {{3,66}, {2,3}, {0,57}};
  g.add_vertex_with_edges(1, edges);
  // edges for 2
  edges = {{3,52}, {1,3}};
  g.add_vertex_with_edges(2, edges);
  // edges for 3
  edges = {{2,52}, {1,66}, {0, 84}};
  g.add_vertex_with_edges(3, edges);
  // edges for 4
  edges = {{0,12}};
  g.add_vertex_with_edges(4, edges);
  // edges for 5
  edges = {};
  g.add_vertex_with_edges(5, edges);
  return g;
}

Graph evil_graph_2() {
  Graph g(4);
  // edges for 0
  vector<Edge> edges = {{2,81}, {3, 24}};
  g.add_vertex_with_edges(0, edges);
  // edges for 1
  edges = {{2,97},{3,38}};
  g.add_vertex_with_edges(1, edges);
  // edges for 2
  edges = {{3,78}, {1,97}, {0,81}};
  g.add_vertex_with_edges(2, edges);
  // edges for 3
  edges = {{2,78}, {1,38}, {0, 24}};
  g.add_vertex_with_edges(3, edges);
  return g;
}

Graph evil_graph_3() {
   fstream graph_input;
   graph_input.open("last_graph_logcopy.txt");
   auto maybe_parsed_g = parse_input(graph_input);
   if (!maybe_parsed_g)
     assert(false);
   return *maybe_parsed_g;
}

void parser_test() {
  ofstream myfile;
  for (int i = 0; i < 180; ++i) {
    auto g = randomGraph(3 + i);
    myfile.open("../example2.txt");
    if (!myfile.is_open()) {
      cout << "could not open file ";
      assert(false);
    }
    myfile << g;
    myfile.close();

    fstream graph_input;
    graph_input.open("../example2.txt");
    if(!graph_input.is_open()){
      cout << "Error open input file" << endl;
      assert(false);
    }

    auto maybe_parsed_g = parse_input(graph_input);
    if (!maybe_parsed_g) {
      graph_input.close();
      cout << "failed to parse graph: " << g;
      assert(false);
    }
    auto&& parsed_g = *maybe_parsed_g;
    if (!(parsed_g == g)) {
      graph_input.close();
      cout << "expected: " << g.edges() << " edges. Got: " << parsed_g.edges();
      cout << "\ninput graph " << g;
      cout << "\n graph parsed " << parsed_g;
      assert(false);
    }
    graph_input.close();
  }
  cout << "success! on good input\n";

  // Try with bad files:
  myfile.open("../example3");
  // malformedgraph.
  auto g = randomGraph(12);
  myfile << "aascxz" << g;
  fstream graph_input;
  graph_input.open("../example3");
    if(!graph_input.is_open()){
      graph_input.close();
      cout << "Error open input file" << endl;
      assert(false);
    }
  assert(!parse_input(graph_input));
}

void test_bellmanford(const Graph& g) {
  auto dists = bellmanford(g, 0);
  cout << g;
  for (int i = 0; i < g.size(); ++i) {
    cout << "d(0," << i << ") -> " << dists[i] << "\n"; 
  }
}

void test_spanner_bellman(const Graph& g) {
  auto distances_g = bellmanford(g, 0);
  auto spanner = three_spanner(g);
  auto distances_spanner = bellmanford(spanner, 0);
  cout << "difference in edges g - span(g) " << g.edges() - spanner.edges() <<
   "\n"; 
  int number_eq = 0;
  int number_not_eq = 0;
  double max_diff = 0;
  std::pair<double, double> max_diff_pair = {0, 0};
  for (int i = 0; i < g.size(); ++i) {
    auto diff = distances_spanner[i] - distances_g[i];
    if (distances_spanner[i] == distances_g[i]) {
      ++number_eq;
    } else {
      ++number_not_eq;
    }
    max_diff = diff > max_diff ? diff : max_diff;
    if (max_diff_pair.first - max_diff_pair.second != max_diff) {
      max_diff_pair.first = distances_spanner[i];
      max_diff_pair.second = distances_g[i];
    }
  }
  cout << "Number of distances not changes: " << number_eq << "\n";
  cout << "Number of distances changed: " << number_not_eq << "\n";
  cout << "Max difference between two distances: " << max_diff << "\n";
  cout << "Max change " << max_diff_pair.second << " -> " <<
    max_diff_pair.first << " \n";
}

void test_spanner_warshall(const Graph& g, bool print_all = false) {
  auto spanner = three_spanner(g);
  auto dists_g = floydwarshall(g);
  auto dists_s = floydwarshall(spanner);
  int num_diff = 0;
  for (int v = 0; v < g.size(); ++v)
    for(int u = v; u < g.size(); ++u) {
      if (dists_g[u][v] != dists_s[v][u] || print_all) {
        cout << "dist_g[" << v << ", " << u << "] = " << dists_g[v][u] << "  ";
        cout << "dist_s[" << v << ", " << u << "] = " << dists_s[v][u] << endl;
        ++num_diff;
      }
    }
  auto expected_edges = g.size() * sqrt(g.size());
  cout << "diff: " << num_diff << endl;
  cout << "diff edges " << g.edges() - spanner.edges() << endl;
  cout << "Number of edges in spanner: " << spanner.edges() << endl;
  cout << "Number of edges in graph: " << g.edges() << endl;
  cout << "Expected number of edges: " <<  expected_edges << endl;
  cout << "actual / expected: " << spanner.edges() / expected_edges << endl;
}


void test_random_vs_warshall(int size) {
  auto g = randomGraph(size);
  auto dists_g = floydwarshall(g);
  cout << "g has: " << g.edges() << endl;
  cout << dists_g.size() << endl;
}

void log_last_graph(const Graph& g, const string& prefix = "") {
  ofstream last_graph_log(prefix + "last_graph_log.txt");
  last_graph_log << g;
}

void test_simple_spanner(const Graph& g, int k = 2) {
  //auto expected_edges = g.size() * sqrt(g.size());
  auto spanner = two_k_minus_1_spanner(k, g);
  auto spanner2 = three_spanner(g);
  cout << "Number of edges in spanner using generic: " << spanner.edges() <<
    endl;
  cout << "Number of edges in spanner using 3 span: " << spanner.edges() <<
    endl;
  cout << "Number of edges in graph: " << g.edges() << endl;
  log_last_graph(spanner, "spanner_");
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

auto last_graph() {
  Graph g(5);
  // edges for 0
  vector<Edge> edges = {{3, 179.431}, {2, 156.348}};
  g.add_vertex_with_edges(0, edges);
  // edges for 1
  edges = {};
  g.add_vertex_with_edges(1, edges);
  // edges for 2
  edges = {{0,156.348}};
  g.add_vertex_with_edges(2, edges);
  // edges for 3
  edges = {{4, 115.096}, {0, 179.431}};
  g.add_vertex_with_edges(3, edges);
  // edges for 4
  edges = {{3,115.096}};
  g.add_vertex_with_edges(4, edges);
  return g;
}

bool check_disconnection(const Graph& g, const Graph& spanner) {
  auto dsts_g = floydwarshall(g);
  auto dsts_s = floydwarshall(spanner); 
      for (int i = 0; i < g.size(); ++i) {
        for(int j = i + 1; j < g.size(); ++j) {
          if (dsts_g[i][j] != std::numeric_limits<double>::infinity() &&
              dsts_s[i][j] == std::numeric_limits<double>::infinity()) {
              cout << "found bad pair " << i << ", " << j << endl;
              return true;
          }
        }
      }
  return false;
}

int main(int argc, char** argv) {
  std::ofstream out("/dev/null");
  std::thread t1( [&] () {
      CreateSpannerStretchReport([](const auto& g) {
          return three_spanner(g);},
          10, 50, 10, "3_spanner_B"); });
  std::thread t2( [&] () {
      CreateSpannerStretchReport([&] (auto&& g) {
          return two_k_minus_1_spanner(5, g, out);},
          10, 50, 10, "5k_A"); });
  t1.join();
  t2.join();
  return 0;
}
