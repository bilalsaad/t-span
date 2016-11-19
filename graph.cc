#include "graph.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <numeric>
#include <fstream>
#include <string>
#include <limits>

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

  Graph randomGraph(int num_v) {
    scoped_timer st(
        "building graph with " + std::to_string(num_v) + " vertices");
    Graph result(num_v);
    for (int i = 0; i < num_v; ++i) {
      for (int j = i + 1; j < num_v; ++j) {
        // Flip a coin to decide wether to add edge <i, j>
        if (random_real() < 0.5) {
          result.add_edge(i, j, static_cast<double>((std::rand() % 100) + 90));
        }
      }
    }
    return result;
  }

  namespace {
    // return the number of vertex
    int head_vertex(const string& line){
      int left_square_bracket = line.find('[');
      int right_square_bracket = line.find(']');
      return stoi(line.substr(left_square_bracket + 1,
            right_square_bracket - left_square_bracket - 1 ));
    }

    // split string by delimiter into sring tokens and return vector of
    // the tokens 
    vector<string> split_by_delimiter(const string& s, const char& c){
      string buff;// = new string();
      vector<string> v;
      for(auto n:s){
        if(n != c) buff+=n; else
          if(n == c && buff != "") { v.push_back(buff); buff = ""; }
      }
      if(buff != "") v.push_back(buff);

      return v;
    }

    //process_vertex takes the line that represents single vertex meta-data
    //and parse each edge (struct) in vector.
    //meta data include the neighbor vertex and the wight of each their edge.
    vector<Edge> process_vertex(const string& vertex_line){
      int left_bracket, right_bracket, comma;
      int neighbor_vertex,edge_wight;
      vector<Edge> edges_vecor;
      left_bracket = vertex_line.find('{');
      right_bracket = vertex_line.find('}');
      string edges = vertex_line.substr(left_bracket + 1,
          right_bracket - left_bracket - 1);
      vector<string> v = split_by_delimiter(edges,'.');
      for(const string& n:v){
        left_bracket = n.find('(');
        right_bracket = n.find(')');
        comma = n.find(',');
        neighbor_vertex = std::stoi(n.substr(left_bracket+1,
              comma - left_bracket - 1));
        edge_wight = std::stoi(n.substr(comma+1, right_bracket - comma - 1));
        edges_vecor.push_back({neighbor_vertex, edge_wight});
      }
      return edges_vecor;
    }
  }

  //takes fstream input and parse it and return Graph in success 
  std::experimental::optional<Graph> parse_input(std::fstream& graph_input){
    string line;
    int head;
    try {
      if(graph_input.is_open()){
        getline(graph_input,line);
        if (line.size() == 0) {
          cout << "got empty line." << "\n";
          return {};
        }
        Graph result(std::stoi(line));
        while(getline(graph_input,line)){
          result.add_vertex_with_edges(head_vertex(line), process_vertex(line));
        }
        return result;
      }
      else {
        cout << "Error open input file" << endl;
        return {};
      }
    } catch (const std::exception& ex) {
      cout << ex.what();
      return {};
    } catch (...) {
      cout << "unexpected error ";
      return {};
    }
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
/*
 void floydWarshall() {
       for( int k = 0; k < n; k++ )
       for( int i = 0; i < n; i++ )
       for( int j = 0; j < n; j++ )
           graph[i][j] = min( graph[i][j], graph[i][k] + graph[k][j] );
   }
*/

vector<vector<double>> floydwarshall(const Graph& g) {
  scoped_timer st("floydwarshall");
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
}  // namespace graphs
