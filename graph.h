#include<vector>


namespace graphs {

// Represents an edge ending in vertex end, with weight w.
struct Edge {
  int end;
  double w;
}

// A class representing a graph, i.e a pair <V,E> of vertices and edges.
// This implementation is an adjacency lists. Vertices are associated with a
// number between 0 and |V|-1. Each edge will have a weight as well.
class Graph {
  private:
    // An adjaceny list for the graph adjlist[i] is a list of all of vertex i's
    // neighbors.
    std::vector<std::vector<Edge>> adj_list;

  public:
    size_t NumVerticies() const { return adj_list.size();}
};

} // namespace graphs.
