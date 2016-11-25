// TODO this file
#include "three-spanner-algorithm.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>
#include <chrono>
using namespace std;
using namespace graphs;


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

void log_last_graph(const Graph& g) {
  ofstream last_graph_log;
  last_graph_log.open("last_graph_log.txt");
  last_graph_log << g;
  last_graph_log.close();
}

void test_simple_spanner(const Graph& g) {
  auto expected_edges = g.size() * sqrt(g.size());
  auto spanner = three_spanner(g);
  cout << "Number of edges in spanner: " << spanner.edges() << endl;
  cout << "Number of edges in graph: " << g.edges() << endl;
  cout << "Expected number of edges: " <<  expected_edges << endl;
  cout << "actual / expected: " << spanner.edges() / expected_edges << endl;
}
int main(int argc, char** argv) {
  //auto g = evil_graph_2();
  //parser_test();
  //parser_test();
  auto g = randomGraph(argc > 1 ? std::stoi(*(argv+1)) : 5);
  //test_spanner_warshall(g, g.size() < 500);
  test_spanner_warshall(g);
  //auto g = evil_graph_3();
  if (g.size() < 1000) log_last_graph(g);
  return 0;
}
