// TODO add IFDEF thing
#include <vector>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <numeric>
namespace graphs {
// Represents an edge ending in vertex end, with weight w.
struct Edge {
  int end;
  double w;
};

// A class representing a graph, i.e a pair <V,E> of vertices and edges.
// This implementation is an adjacency lists. Vertices are associated with a
// number between 0 and |V|-1. Each edge will have a weight as well.
class Graph {
  private:
    // An adjaceny list for the graph adjlist[i] is a list of all of vertex i's
    // neighbors.
    std::vector<std::vector<Edge>> adj_list;

  public:
    int size() const { return adj_list.size();}
    void AddV(int v, std::vector<Edge> neighbors) {
      if (adj_list.size() <= v) {
        adj_list.resize(v + 1);
      }
      adj_list[v] = std::move(neighbors);
    }
    const std::vector<Edge>& Neighbors(int v) const {return adj_list[v];}
    Graph(int size): adj_list(size) {}
    Graph() {}

    long edges() {
      return std::accumulate(std::begin(adj_list), std::end(adj_list), 0.0,
          [] (long acc, auto&& next) { return acc += next.size();});
    }
};

Graph randomGraph(int num_v) {
  Graph result(num_v);
  std::srand(std::time(0));
  for (int i = 0; i < num_v; ++i) {
    std::vector<Edge> edges;
    for (int j = 0; j < num_v; ++j) {
       if (i == j) {
          continue;
       }
       // Flip a coin to decide wether to add edge <i, j>
       if ((std::rand() % 2) == 1) {
         edges.push_back({j, 1});
       }
    }
    result.AddV(i, edges);
  }

  return result;
}

std::ostream& operator<<(std::ostream& os, const Edge& g) {
  os << "(" << g.end << ", " << g.w << ")";
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
} // namespace graphs.
