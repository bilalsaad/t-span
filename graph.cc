#include "graph.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <numeric>
namespace graphs {
Graph randomGraph(int num_v) {
  Graph result(num_v);
  std::srand(std::time(0));
  for (int i = 0; i < num_v; ++i) {
    for (int j = i + 1; j < num_v; ++j) {
       // Flip a coin to decide wether to add edge <i, j>
       if ((std::rand() % 100) < 64) {
         result.add_edge(i, j, static_cast<double>(std::rand() % 100));
       }
    }
  }
  return result;
}

std::ostream& operator<<(std::ostream& os, const Edge& g) {
  os << "(" << g.end << "," << g.w << ")";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Graph& g) {
  for (int i = 0; i < g.size(); ++i) {
    os << "[" << i << "]->{";
    for (const auto& e : g.Neighbors(i)) {
      os << e;
    }
    os << "}\n";
  }
  return os;
}
}  // namespace graphs
