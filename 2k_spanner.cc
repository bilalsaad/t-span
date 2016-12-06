#include "2k_spanner.h"
#include "util.h"
#include "graph.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <functional>

namespace graphs {
  using util::scoped_timer;
  using util::random_real;
namespace{
  using Clusters = vector<int>;
  
  auto sample (const vector<int>& cluster_representives, double probability) {
    std::unordered_set<int> sampled_clusters;

    for (auto rep : cluster_representives) {
      if (random_real() < probability) {
        sampled_clusters.emplace(rep);
      }
    }

    return sampled_clusters;
  }

  vector<int> numbers_to_n(int n) {
    vector<int> res(n);
    for (int i = 0; i < n; ++i)
        res[i] = i;
    return res;
  }

  // returns all the vertices belonging to cluster_names from clusters.
  vector<int> add_vertices_from_clusters(
      const unordered_set<int>& cluster_names,
      const Clusters& clusters) {
    vector<int> result;
    for (int vertex = 0; vertex < clusters.size(); ++vertex) {
       if (cluster_names.count(clusters[vertex]) != 0)
         result.push_back(vertex); 
    }
    return result;
  }

  // Creates clusters corresponding to the cluster_names from clusters.
  Clusters create_clusters_from_samples(
      const unordered_set<int>& cluster_names,
      const Clusters& clusters) {
    Clusters result(clusters.size());
    for (int vertex = 0; vertex < clusters.size(); ++vertex){
      if (cluster_names.count(clusters[vertex]) != 0) {
        result[vertex] = clusters[vertex];
      } else {
        result[vertex] = -1;
      }
    }
    return result;
  }


  struct ClusterAndEdge {
    int cluster; 
    Edge min_edge;
  };

  bool operator>(const ClusterAndEdge& a, const ClusterAndEdge& b) {
    return a.min_edge.w > b.min_edge.w;
  }
  bool operator<(const ClusterAndEdge& a, const ClusterAndEdge& b) {
    return a.min_edge.w < b.min_edge.w;
  }

  auto create_cluster_to_min_edge_heap(
      const Graph& g,
      int vertex,
      const Clusters& clusters) {
    std::unordered_map<int, Edge> cluster_representives;
    for (const auto& e : g.neighbors(vertex)) {
      int cluster = clusters[e.end];
      auto current_rep = cluster_representives.find(cluster);
      if (current_rep == std::end(cluster_representives)) {
        cluster_representives.emplace(clusters[cluster], e);
      } else if (current_rep->second > e) {
        current_rep->second = e;
      }
    }
    vector<ClusterAndEdge> result;
    std::transform(std::begin(cluster_representives),
                   std::end(cluster_representives),
                   std::back_inserter(result),
                   [] (const auto& rep_and_edge) -> ClusterAndEdge {
                    return {rep_and_edge.first, rep_and_edge.second};
                   });
    std::make_heap(std::begin(result), std::end(result),
        [] (auto&& a, auto&& b) { return a > b;});
    return result;
  }

  bool is_sampled(int vertex, const std::unordered_set<int>& samples,
                  const Clusters& clusters) {
    return samples.count(clusters[vertex]) != 0;
  }

  template<typename T, typename Container>
  T get_top(Container&& heap) {
    std::pop_heap(std::begin(heap), std::end(heap));
    T min_edge = heap.back();
    heap.pop_back();
    return min_edge;
  } 

  auto form_clusters(Graph g, Graph& spanner, int k) {
    auto V_i= numbers_to_n(g.size());
    auto C_i = numbers_to_n(g.size());
    vector<int> V_i_next;
    Clusters C_i_next;
    double probability = pow(g.size(), -1.0 / static_cast<double>(k));
    cout << probability << endl;
    for (int i = 0; i < (k / 2); ++i) {
      auto R_i = sample(C_i, probability);
      V_i_next = add_vertices_from_clusters(R_i, C_i);
      C_i_next = create_clusters_from_samples(R_i, C_i);

      for (auto v : V_i) {
        // Only iterate non vertices not in sampled clusters.
        if (is_sampled(v, R_i, C_i)) {
          cout << "Skipping vertex: " << v << endl;
          continue;
        }
        auto min_edge_heap =
          create_cluster_to_min_edge_heap(g, v, C_i);
        // If the vertex has no adjacent sampled clusters, we add the minimum
        // edge of all of its neighbors.
        if (std::none_of(std::begin(g.neighbors(v)),
              std::end(g.neighbors(v)),
              [&] (auto&& neighbor) {
              return is_sampled(neighbor.end, R_i, C_i);})) {
          g.clear_neighbors(v);

          while (!min_edge_heap.empty()) {
            auto min_edge = get_top<ClusterAndEdge>(min_edge_heap); 
            spanner.add_edge(v, min_edge.min_edge.end, min_edge.min_edge.w);
          }
        } else {  // V is adjacent to a sampled cluster.
          // In this case we add the minimum edges corresponding to the adjacent
          // clusters until we reach a sampled one. Removing all the edges of
          // the clusters we add.
          int cluster = -1;
          while (!min_edge_heap.empty() && cluster != -1) {
            auto min_edge = get_top<ClusterAndEdge>(min_edge_heap); 
            auto end_point = min_edge.min_edge.end;
            // Now we remove all edges corresponding to this cluster.
            g.remove_neighbors(v, [&] (int neighbor) {
                return C_i[neighbor] == min_edge.cluster;});
            spanner.add_edge(v, end_point, min_edge.min_edge.w);
            if (is_sampled(end_point, R_i, C_i)) {
              cluster = min_edge.cluster;
            }
          }
          C_i_next[v] = cluster;
          V_i_next.push_back(v);
        } 
      }

      // Remove intra cluster edgez.
      for (int vertex = 0; vertex < g.size(); ++vertex) {
        if (C_i_next[vertex] != -1) {
          g.remove_neighbors(vertex, [&] (auto&& u) {
              return C_i_next[vertex] == C_i_next[u];});
        }
      }

      // Next iteration initializations.
      C_i = std::move(C_i_next);
      V_i = std::move(V_i_next);
    }

    return std::make_pair(g, C_i);
  }


  template<typename Container, typename Key, typename Value, typename KeyPred,
            typename ValuePred>
  void replace_if_pred_in_map(Container& container, Key&& key, Value&& v,
                              KeyPred&& key_pred, ValuePred&& value_pred) {
    auto iter = std::find_if(std::begin(container),
                          std::end(container),
                          [&] (auto&& a) {
                            return key_pred(a.first, key) &&
                            value_pred(a.second, v);});
    if (iter == std::end(container)) {
      container.emplace(key, v);
    } else if (value_pred(iter->second, v)) {
      iter->second = v;
    }
  }

  // Assume this is the case of odd k..
  auto join_clusters(const Graph& not_yet_added, Graph& spanner,
      const Clusters& clusters) {

    auto pair_hash = [] (const std::pair<int, int>& pair) -> size_t {
      return pair.first ^ pair.second;
    };

    auto compare_pairs = [] (const std::pair<int, int>& a,
                           const std::pair<int, int>& b) {
      return (a.first == b.first && b.second == a.second) ||
             (a.second == b.first && a.first == b.second);
    };

    struct ExtendedEdge {
      int u, v;
      double w;
      ExtendedEdge(int u, int v, double w): u(u), v(v), w(w) {}
      ExtendedEdge(const ExtendedEdge& e): u(e.u), v(e.v), w(e.w) {}
    };

    std::unordered_map<std::pair<int, int>,
                        ExtendedEdge,
                        decltype(pair_hash),
                        decltype(compare_pairs)>
                          map(1, pair_hash, compare_pairs);
    // Now we iterate over all the edges in not_yet_added maintaining the min
    // edge for each pair of clusters.
    for (int v = 0; v < not_yet_added.size(); ++v) {
      if (not_yet_added.neighbors(v).empty() || clusters[v] == -1)
         continue;
      int v_cluster = clusters[v];
      for (const auto& edge : not_yet_added.neighbors(v)) {
        //assert(clusters[edge.end] != v_cluster);
        int neighbor_cluster = clusters[edge.end];
        replace_if_pred_in_map<decltype(map)>(map,
            std::make_pair(v_cluster, neighbor_cluster),
            ExtendedEdge(v, edge.end, edge.w), compare_pairs,
            [&] (const ExtendedEdge& old, const ExtendedEdge& new_key) {
                return old.w > new_key.w; 
                }
            );
       }
    }

    for(const auto& e : map) {
      spanner.add_edge(e.second.u, e.second.v, e.second.w);
    }
  }
}  // namespace.


Graph two_k_minus_1_spanner(int k, Graph g) {
  Graph spanner(g.size());
  auto end_of_phase_1 = form_clusters(g, spanner, k);
  cout << "The spanner edges: " << spanner.edges() << endl;
  join_clusters(end_of_phase_1.first,spanner, end_of_phase_1.second);
  cout << "The spanner: " << spanner << endl;
  cout << "The spanner edges: " << spanner.edges() << endl;
  cout << "The g edges: " << g.edges() << endl;
  //cout << "The g : " << g << endl;
  return spanner;
}

}  // namespace graphs
