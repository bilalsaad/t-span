#include "graph.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <numeric>
#include <fstream>

namespace graphs {
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

  namespace {
    int headG(const string& line){
      int a = line.find('[');
      int b = line.find(']');
      return stoi(line.substr(a+1,b-a-1));
    }

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

    vector<Edge> process_line(const string& line){
      int a,b,c;
      int num1,num2;
      vector<Edge> edges_vecor;
      a = line.find('{');
      b = line.find('}');
      string edges = line.substr(a+1,b-a-1);
      // explode(edges);
      vector<string> v = split_by_delimiter(edges,'.'); // edges vector
      for(const string& n:v){
        a = n.find('(');
        b = n.find(')');
        c = n.find(',');
        num1 = std::stoi(n.substr(a+1,c-a-1));
        num2 = std::stoi(n.substr(c+1,b-c-1));
        edges_vecor.push_back({num1,num2});
      }
      return edges_vecor;
    }
  }  //namespace

  std::experimental::optional<Graph> parse_input(string input){
    string line;
    int head;
    try {
      ifstream graph_input(input);
      if(graph_input.is_open()){
        getline(graph_input,line);
        if (line.size() == 0) {
          cout << "got empty line for file:" << input << "\n";
          return {};
        }
        Graph result(std::stoi(line));
        while(getline(graph_input,line)){
          result.add_vertex_with_edges(headG(line), process_line(line));
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
}  // namespace graphs
