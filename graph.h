#ifndef GRAPH_H
#define GRAPH_H
#include <vector>
#include <numeric>
#include <functional>
namespace graphs {
// Represents an edge ending in vertex end, with weight w.
struct Edge {
  int end;
  double w;
  Edge(int end, int w) : end(end), w(w) {}
  friend bool operator<(const Edge& a, const Edge& b) {return a.w < b.w;}
  friend bool operator>(const Edge& a, const Edge& b) {return a.w > b.w;}
  friend bool operator==(const Edge& a, const Edge& b) {
    return a.w == b.w && a.end == b.end;
  }
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
    /*
    void AddV(int v, const std::vector<Edge>& neighbors) {
      if (adj_list.size() <= v) {
        adj_list.resize(v + 1);
      }
      adj_list[v].insert(std::end(adj_list[v]), std::begin(neighbors),
                         std::end(neighbors));
    } */
    
    void add_vertex_with_edges(int v, const std::vector<Edge>& neighbors) {
      for (auto&& e : neighbors) {
         add_edge(v, e.end, e.w);
      }
    }

    const std::vector<Edge>& Neighbors(int v) const {return adj_list[v];}
    void add_edge(int u, int v, double w) {
      adj_list[u].emplace_back(v, w);
      adj_list[v].emplace_back(u, w);
    }
    Graph(int size): adj_list(size) {}
    Graph() {}

    long edges() {
      return std::accumulate(std::begin(adj_list), std::end(adj_list), 0.0,
          [] (long acc, auto&& next) { return acc += next.size();});
    }
    // Returns a graph which is g without all the edges in g s.t pred(e)=true.
    template<typename Pred>
    friend Graph filter_edges(Graph g, Pred&& pred) {
      for (int vertex = 0; vertex < g.size(); ++vertex) {
        auto& neighbors = g.adj_list[vertex];
        neighbors.erase(std::remove_if(std::begin(neighbors),
              std::end(neighbors), [&pred, vertex] (const Edge& e) {
              return pred(vertex, e.end);
              }),
            std::end(neighbors));
      }
      return g;
    }
};

// Generates a random graph with n vertices.
Graph randomGraph(int n);


std::ostream& operator<<(std::ostream& os, const Edge& g);
std::ostream& operator<<(std::ostream& os, const Graph& g);
} // namespace graphs.
#endif
