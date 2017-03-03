#include "graph.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <numeric>
#include <fstream>
#include <string>
#include <limits>

using namespace std;
namespace graphs {
  using scoped_timer = util::scoped_timer;
  using util::random_real;
  namespace {
    // Returns true if a is a subgraph of b. i.e for all edges e in a they exist
    // in b.
    bool is_included(const Graph& a, const Graph& b) {
      for (int i = 0; i < a.size(); ++i) {
        for (auto&& edge : a.neighbors(i)) {
          if (!b.has_edge(i, edge.end)) {
            return false;
          }
        }
      }
      return true;
    }
  } // namespace
  bool operator==(const Graph& a, const Graph& b) {
    return a.size() == b.size() && is_included(a, b) && is_included(b, a);
  }

  Graph randomGraph(int num_v, double edge_density,
      const std::function<double(void)>& edge_weight) {
    //scoped_timer st(
    //    "building graph with " + std::to_string(num_v) + " vertices");
    Graph result(num_v);
    for (int i = 0; i < num_v; ++i) {
      for (int j = i + 1; j < num_v; ++j) {
        // Flip a coin to decide wether to add edge <i, j> according to density.
        if (random_real() < edge_density) {
          result.add_edge(i, j, edge_weight());
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
    os << g.size() << "\n";
    for (int i = 0; i < g.size(); ++i) {
      os << "[" << i << "]->{";
      for (const auto& e : g.neighbors(i)) {
        os << e << ".";
      }
      os << "}\n";
    }
    return os;
  }

  vector<double> bellmanford(const Graph& g, int src) {
    vector<double> distances(g.size(), std::numeric_limits<double>::infinity());
    vector<int> predecessors(g.size(), -1);

    distances[src] = 0;
    for (int i = 0; i < g.size(); ++i) {
      // to go over the edges we go over the vertices.
      for(int v = 0; v < g.size(); ++v) {
        for (const auto& edge : g.neighbors(v)) {
          if (distances[v] + edge.w < distances[edge.end]) {
            distances[edge.end] = distances[v] + edge.w;
            predecessors[edge.end] = v;
          }
        }
      }
    }
    // TODO(check negative cycles)
    for(int v = 0; v < g.size(); ++v) {
      for (const auto& edge : g.neighbors(v)) {
        if (distances[v] + edge.w < distances[edge.end]) {
          cout << "Graph contains cycles of legative_length";
        }
      }
    }
    return distances;
  }

  vector<vector<double>> floydwarshall(const Graph& g) {
    //scoped_timer st("floydwarshall");
    vector<vector<double>> dists (g.size(),
        vector<double>(g.size(), std::numeric_limits<double>::infinity()));
    // Initialize the dists with the edge weight for the graph.
    for (int i = 0; i < g.size(); ++i) {
      dists[i][i] = 0;
    }
    for (int i = 0; i < g.size(); ++i) {
      for(int j = 0; j < g.size(); ++j) {
        if (g.has_edge(i, j)) {
          // put its weight in i , j;
          for (const auto& e : g.neighbors(i)) {
            if (e.end == j) {
              dists[i][j] = e.w;
              break;
            }
          }
        }
      }
    }

    for (int k = 0; k < g.size(); ++k)
      for (int i = 0; i < g.size(); ++i)
        for (int j = 0; j < g.size(); ++j)
          dists[i][j] = std::min(dists[i][j], dists[i][k] + dists[k][j]);
    return dists;
  }

  // TODO: Using floydwarshall to check this is VERY slow, should do a dfs or 
  // bfs.
  bool check_subgraph_disconnection(const Graph& g, const Graph& subgraph) {
    auto dsts_g = floydwarshall(g);
    auto dsts_s = floydwarshall(subgraph); 
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

  void Graph::clear_neighbors(int v) {
    for (const auto& edge : adj_list[v]) {
      adj_list[edge.end].erase({v, edge.w}); 
    }
    adj_list[v].clear();
  }
}  // namespace graphs
