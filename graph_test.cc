// TODO this file
#include "three-spanner-algorithm.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>
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

void parser_test() {
  ofstream myfile;
  fstream graph_input;
  for (int i = 0; i < 180; ++i) {
    auto g = randomGraph(3 + i);
    myfile.open("../example2.txt");
    if (!myfile.is_open()) {
      cout << "could not open file ";
      assert(false);
    }
    myfile << g;
    myfile.close();

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
  }
  cout << "success! on good input\n";
  graph_input.close();

  // Try with bad files:
  myfile.open("../example3");
  // malformedgraph.
  auto g = randomGraph(12);
  myfile << "aascxz" << g;
  graph_input.open("../example3");
    if(!graph_input.is_open()){
      graph_input.close();
      cout << "Error open input file" << endl;
      assert(false);
    }
  assert(!parse_input(graph_input));
}

int main(int argc, char** argv) {
  //auto g = randomGraph(argc > 1 ? std::stoi(*(argv+1)) : 5);
  //auto g = evil_graph_2();
  //parser_test();
  parser_test();
  
  return 0;
 }
