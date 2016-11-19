#ifndef GRAPH_H
#define GRAPH_H
#include <vector>
#include <numeric>
#include <functional>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <chrono>
#include <experimental/optional>
#include "util.h"

using namespace std;

namespace graphs {
  // Represents an edge ending in vertex end, with weight w.
  struct Edge {
    int end;
    double w;
    Edge(int end, int w) : end(end), w(w) {}
    Edge(const Edge& other): end(other.end), w(other.w) {}
    friend bool operator<(const Edge& a, const Edge& b) {return a.w < b.w;}
    friend bool operator>(const Edge& a, const Edge& b) {return a.w > b.w;}
    friend bool operator==(const Edge& a, const Edge& b) {
      return a.end == b.end;
    }
  };

} // namespace graphs
// Add a hash for an edge - we just hash the end vertice.
namespace std {
  template<>
    struct hash<graphs::Edge> {
      std::size_t operator()(const graphs::Edge& e) const {
        using std::hash;
        return hash<int>()(e.end);
      }
    };
}  // namespace std.

namespace graphs {
  // A class representing a graph, i.e a pair <V,E> of vertices and edges.
  // This implementation is an adjacency lists. Vertices are associated with a
  // number between 0 and |V|-1. Each edge will have a weight as well.
  class Graph {
    private:
      // An adjaceny list for the graph adjlist[i] is a list of all of vertex is
      // neighbors.
      std::vector<std::unordered_set<Edge>> adj_list;

    public:
      Graph(int size): adj_list(size) {}
      Graph() {}
      int size() const { return adj_list.size();}

      template<typename Container>
        void add_vertex_with_edges(int v, Container&& neighbors) {
          for (auto&& e : neighbors) {
            add_edge(v, e.end, e.w);
          }
        }

      const std::unordered_set<Edge>& neighbors(int v) const{
        return adj_list[v];}

      void add_edge(int u, int v, double w) {
        adj_list[u].emplace(v, w);
        adj_list[v].emplace(u, w);
      }

      bool has_edge(int v, int u) const {
        return v < size() && u < size() &&
          (adj_list[v].count({u, 0}) != 0 || adj_list[u].count({v, 0}) != 0);
      }

      long edges() const {
        return std::accumulate(std::begin(adj_list), std::end(adj_list), 0,
            [] (long acc, auto&& next) { return acc += next.size();});
      }

      // Returns a graph which is g without all the edges in g s.t pred(e)=true.
      // TODO(): if the predicate is not symmetric this may turn an undirected
      // graph to a directed one..
      template<typename Pred>
        friend Graph filter_edges(Graph g, Pred&& pred) {
          util::scoped_timer st("filter_edges");
          for (int vertex = 0; vertex < g.size(); ++vertex) {
            auto& neighbors = g.adj_list[vertex];
            std::unordered_set<Edge> temp;
            for (const auto& edge : neighbors) {
              if (!pred(vertex, edge.end)) {
                temp.emplace(edge.end, edge.w);
              }
            }
            neighbors = std::move(temp);
          }
          return g;
        }

  };

  // Generates a random graph with n vertices.
  Graph randomGraph(int n);
  std::experimental::optional<Graph> parse_input(std::fstream&);
  vector<double> bellmanford(const Graph& g, int src);
  vector<vector<double>> floydwarshall(const Graph& g);
  std::ostream& operator<<(std::ostream& os, const Edge& g);
  std::ostream& operator<<(std::ostream& os, const Graph& g);
  bool operator==(const Graph& a, const Graph& b);
} // namespace graphs.
#endif
