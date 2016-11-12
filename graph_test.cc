// TODO this file
#include "three-spanner-algorithm.h"
#include <iostream>
#include <string>
#include <vector>
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

int main(int argc, char** argv) {
  //auto g = randomGraph(argc > 1 ? std::stoi(*(argv+1)) : 5);
  auto g = evil_graph();
  cout << "input graph:\n" << g;
  //auto threespan = three_spanner(g);
  //cout << "three spanner:\n" << threespan;
  return 0;
 }
