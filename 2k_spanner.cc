#include "2k_spanner.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <queue>
#include <functional>
#include <cassert>
#include <iomanip>
#include "util.h"
#include "graph.h"

using namespace std;

namespace graphs {
  using util::scoped_timer;
  using util::random_real;
namespace{
  using Clusters = vector<int>;
  using SampleType = std::unordered_set<int>;
  
  auto sample (const vector<int>& cluster_representives, double probability) {
    std::unordered_set<int> sampled_clusters;
    std::unordered_set<int> all_clusters(std::begin(cluster_representives),
        std::end(cluster_representives));
                    
    for (auto rep : all_clusters) {
      if (rep != -1 && random_real() < probability) {
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
    operator bool() {
      return cluster != -1;
    }
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
        [] (const auto& a, const auto& b) { return a > b;});
    return result;
  }

  // Given a vertex 'vertex' returns a map where for each cluster in 'clusters'
  // there is the value of a minimum edge from 'vertex' to that cluster.
  auto create_cluster_to_min_edge_map(const Graph& g,
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
    return cluster_representives;
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

  void print_iteration_info(int iteration, const Clusters& clusters,
      const SampleType& samples, std::ostream& out) {
    out << "Iteration: " << iteration << std::endl;
    out << "Clusters: " << std::endl;
    std::map<int, std::vector<int>> cluster_elaborate;
    for (int i = 0; i < clusters.size(); ++i) {
      cluster_elaborate[clusters[i]].emplace_back(i);
    }
    for (const auto& cluster : cluster_elaborate) {
      const auto& c = cluster.first;
      const auto& vs = cluster.second;
      out << "  Cluster:" << c << std::endl;
      for (auto&& v : vs) {
        out << v << ", ";
      }
      out << std::endl;
    }

    out << std::endl << " SampledClusters: " << std::endl;
    for (const auto& sample : samples) {
      out << sample << ", "; 
    }
    out << std::endl << "END ITERATION \n\n";
  }

  struct form_cluster_ret {
    Clusters last_clustering;
    Clusters before_last;
    Graph not_added;
  };

  auto form_clusters(Graph g, Graph& spanner, int k, std::ostream& out) {
    auto V_i = numbers_to_n(g.size());
    auto C_i = numbers_to_n(g.size());
    vector<int> V_i_next;
    Clusters C_i_next;
    Clusters C_before_last;
    int edges_added = 0;
    const double& probability = pow(g.size(), -1.0 / static_cast<double>(k));
    for (int i = 0; i < k / 2 ; ++i) {
      auto R_i = sample(C_i, probability);
      V_i_next = add_vertices_from_clusters(R_i, C_i);
      C_i_next = create_clusters_from_samples(R_i, C_i);
      //print_iteration_info(i, C_i, R_i, out);

      for (auto v : V_i) {
        // Only iterate non vertices not in sampled clusters.
        if (is_sampled(v, R_i, C_i)) {
          continue;
        }
        auto cluster_min_edge_map = create_cluster_to_min_edge_map(g, v, C_i);
        // If the vertex has no adjacent sampled clusters, we add the minimum
        // edge of all of its neighbors.
        if (std::none_of(std::begin(g.neighbors(v)),
              std::end(g.neighbors(v)),
              [&] (const auto& neighbor) {
              return is_sampled(neighbor.end, R_i, C_i);})) {
          g.clear_neighbors(v);
          for (auto&& cluster_and_edge : cluster_min_edge_map) {
            ++edges_added;
            spanner.add_edge(v,
                cluster_and_edge.second.end,
                cluster_and_edge.second.w);
          } 
        } else {  // V is adjacent to a sampled cluster.
          ClusterAndEdge sentinel_edge =
            {-1, {-1, std::numeric_limits<double>::max()}};
          auto best_sampled = std::accumulate(std::begin(cluster_min_edge_map),
              std::end(cluster_min_edge_map), sentinel_edge,
              [&] (auto&& best_sampled, auto&& next) {
                if (is_sampled(next.second.end, R_i, C_i)
                    && next.second < best_sampled.min_edge)
                    return ClusterAndEdge{next.first, next.second}; 
                return best_sampled;
              });
          assert(best_sampled.cluster != sentinel_edge.cluster);
          C_i_next[v] = best_sampled.cluster;
          V_i_next.push_back(v);
          for (const auto& cluster_and_edge : cluster_min_edge_map) {
            const auto& cluster = cluster_and_edge.first;
            const auto& min_edge = cluster_and_edge.second;
            if (cluster == best_sampled.cluster ||
                min_edge < best_sampled.min_edge) {
              // Now we remove all edges corresponding to this cluster.
              g.remove_neighbors(v, [&] (int neighbor) {
                  return C_i[neighbor] == cluster;});
              spanner.add_edge(v, min_edge.end, min_edge.w);
              ++edges_added;
            }
          }

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
      C_before_last = std::move(C_i);
      C_i = std::move(C_i_next);
      V_i = std::move(V_i_next);
    }

    return form_cluster_ret {C_i, C_before_last, g};
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

  auto join_clusters(const Graph& not_yet_added, Graph& spanner,
      const Clusters& last_clustering, const Clusters& before_last_clustering) {
    auto pair_hash = [] (const std::pair<int, int>& pair) -> size_t {
      return pair.first ^ pair.second;
    };

    auto compare_pairs = [] (const std::pair<int, int>& a,
                           const std::pair<int, int>& b) {
      return (a.first == b.first && b.second == a.second) ||
             (a.second == b.first && a.first == b.second);
    };


    std::unordered_map<std::pair<int, int>,
                        ExtendedEdge,
                        decltype(pair_hash),
                        decltype(compare_pairs)>
                          map(1, pair_hash, compare_pairs);
    // Now we iterate over all the edges in not_yet_added maintaining the min
    // edge for each pair of clusters.
    for (int v = 0; v < not_yet_added.size(); ++v) {
      if (not_yet_added.neighbors(v).empty() || last_clustering[v] == -1)
         continue;
      int v_cluster = last_clustering[v];
      for (const auto& edge : not_yet_added.neighbors(v)) {
        //assert(clusters[edge.end] != v_cluster);
        int neighbor_cluster = before_last_clustering[edge.end];
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

namespace {

  // Initialize V_i to 0 ... n-1
  auto initialize_V(const Graph& g) {
    std::unordered_set<int> res;
    for (int i = 0; i < g.size(); ++i)
      res.emplace(i);
    return res;
  }
  // Initialize the clusters to be {{v} | v in G}
  auto initialize_Clusters(const Graph& g) {
    std::unordered_map<int, int> vertex_cluster_map;
    for (int v = 0;  v < g.size(); v++) {
      vertex_cluster_map.emplace(v, v); 
    }
    return vertex_cluster_map;
  }

  auto sample_clusters(const std::unordered_map<int, int>& clusters, int k, int n) {
    // The probability of sampling a cluster.
    const double probability = pow(n, -1.0 / static_cast<double>(k));

    std::unordered_set<int> sampled_clusters;
    std::unordered_set<int> all_clusters;
    for (auto&& vertex_cluster : clusters) {
      all_clusters.emplace(vertex_cluster.second);
    }
                    
    for (auto rep : all_clusters) {
      if (random_real() < probability) {
        sampled_clusters.emplace(rep);
      }
    }

    return sampled_clusters;
  }

  // Return true if v's cluster is sampled.
  inline bool is_vertex_sampled(const std::unordered_set<int>& samples,
                         const std::unordered_map<int,int>& clusters,
                         int v) {
    auto vertex_entry = clusters.find(v);
    // Vertex must exist.
    assert(vertex_entry != std::end(clusters));
    return samples.count(vertex_entry->second) > 0;
  }

  // Returns all the vertices in sampled clusters.
  auto vertices_from_sampled_clusters(
      const std::unordered_set<int>& sampled_clusters,
      const std::unordered_map<int, int>& clusters) {
    std::unordered_set<int> v_i_next;
    for (auto&& vertex_cluster : clusters) {
      if (sampled_clusters.count(vertex_cluster.second) > 0)
        v_i_next.emplace(vertex_cluster.first);
    }
    return v_i_next;
  }

  // Returns a clustering, that consists only of the sampled clusters
  auto clusters_from_sample(const std::unordered_map<int, int>& prev_clusters,
                            const std::unordered_set<int>& samples) {
    std::unordered_map<int, int> new_clusters;
    std::copy_if(std::begin(prev_clusters),
                 std::end(prev_clusters),
                 std::inserter(new_clusters, std::end(new_clusters)),
                 [&samples] (const auto& vertex_cluster) {
                   return samples.count(vertex_cluster.second) > 0;
                 });
    return new_clusters;
  }

  // Given a vertex v in g, a map of cluster->min_edge s.t min_edge is the
  // nearest vertex u in cluster 'c' to v.
  std::unordered_map<int, Edge> cluster_to_min_edge_map(
      const Graph& g,
      int vertex,
      const std::unordered_map<int, int>& clusters) {
    std::unordered_map<int, Edge> cluster_representives;
    for (const auto& e : g.neighbors(vertex)) {
      int cluster = clusters.find(e.end)->second;
      auto current_rep = cluster_representives.find(cluster);
      if (current_rep == std::end(cluster_representives)) {
        cluster_representives.emplace(clusters.find(e.end)->second, e);
      } else if (current_rep->second > e) {
        current_rep->second = e;
      }
    }
    return cluster_representives;
  }

  struct form_cluster2_ret {
    std::unordered_map<int, int> last_clustering, before_last;
    Graph not_added;
    std::unordered_set<int> remaining_vertices;
  };


  // If v has a sampled neighbor, the nearest one is returned. If v has no such
  // neighbors 'empty' is returned.
  ClusterAndEdge nearest_sampled_neighbor(
      const Graph& g, int vertex,
      const std::unordered_set<int>& sampled_clusters,
      const std::unordered_map<int, int>& clustering,
      const std::unordered_map<int, Edge>& cluster_min_edge_map) {
    ClusterAndEdge sentinel_edge =
    {-1, {-1, std::numeric_limits<double>::max()}};
    return std::accumulate(std::begin(cluster_min_edge_map),
        std::end(cluster_min_edge_map), sentinel_edge,
        [&] (auto&& best_sampled, auto&& next) {
        if (is_vertex_sampled(sampled_clusters, clustering, next.second.end)
            && next.second < best_sampled.min_edge) {
            return ClusterAndEdge{next.first, next.second}; 
        }
        return best_sampled;
        });
  }

  //
  auto form_clusters_2(Graph g, Graph& spanner, int k, int iters) {
    // Maps each vertex to its cluster.
    std::unordered_set<int> V_i = initialize_V(g);
    // Clusters are represented as an array of size |V| where C_i[u] points to
    // center of the cluster, vertex u is in.
    auto C_i = initialize_Clusters(g);
    // These represent the vertices and the clusters that will be used in the
    // next iteration.
    decltype(V_i) V_i_next;
    decltype(C_i) C_i_next;
    decltype(C_i) C_before_last;
    for (int i = 0; i < iters ; ++i) {
      // Sample clusters from C_i for this iteration.
      auto R_i = sample_clusters(C_i, k, g.size());
      // Initialize V_{i+1} and C_i_{i+1}
      V_i_next = vertices_from_sampled_clusters(R_i, C_i);
      C_i_next = clusters_from_sample(C_i, R_i);
      // Now we process each vertex in V_i, not belonging to any sampled cluster
      for (int v : V_i) {
        if (is_vertex_sampled(R_i, C_i, v))
          continue;
        auto cluster_min_edge_map = cluster_to_min_edge_map(g, v, C_i);
        auto maybe_best_sampled = nearest_sampled_neighbor(
            g, v, R_i, C_i, cluster_min_edge_map);
        if (!maybe_best_sampled) {
          g.clear_neighbors(v);
          for (const auto& cluster_and_edge : cluster_min_edge_map) {
            const auto& edge = cluster_and_edge.second;
            spanner.add_edge(v, edge.end, edge.w);
          }
        } else {  // v is adjacent to sampled vertices.
          const auto& best_sampled = maybe_best_sampled;
          C_i_next.emplace(v, best_sampled.cluster);
          V_i_next.emplace(v);
          for (const auto& cluster_and_edge : cluster_min_edge_map) {
            const auto& cluster = cluster_and_edge.first;
            const auto& min_edge = cluster_and_edge.second;
            if (cluster == best_sampled.cluster ||
                min_edge < best_sampled.min_edge) {
              // Now we remove all edges corresponding to this cluster.
              g.remove_neighbors(v, [&] (int neighbor) {
                  return C_i[neighbor] == cluster;});
              spanner.add_edge(v, min_edge.end, min_edge.w);
            }
          } 
        }
      }
      // Now we need to remove the intra cluster edges.
      for (int vertex : V_i_next) {
        g.remove_neighbors(vertex,
            [&] (auto&& u) { return C_i_next[vertex] == C_i_next[u]; });
      }
      C_before_last = std::move(C_i);
      C_i = std::move(C_i_next);
      V_i = std::move(V_i_next);
    }
  return form_cluster2_ret {C_i, C_before_last, g, V_i};
}


// maps <c1, c2> -> edge, s.t edge is the minimum edge between c1 and c2
// in 'remaining'.
auto cluster_pair_maps(const Graph& remaining,
    const std::unordered_set<int>& remaining_vertices,
    const std::unordered_map<int,int>& c1,
    const std::unordered_map<int,int>& c2) {
  std::unordered_map<int, std::unordered_map<int, ExtendedEdge>> map;
  for (int vertex : remaining_vertices) {
    int v_cluster = c1.find(vertex)->second;
    for (const auto& edge : remaining.neighbors(vertex)) {
      // If this edge exists edge.end must be in a cluster.
      int neighbor_cluster = c2.find(edge.end)->second;
      // this is bit hacky, to make sure we are symmetric.
      auto& neighbor_entry_a = map[v_cluster][neighbor_cluster];
        auto& neighbor_entry_b = map[neighbor_cluster][v_cluster];
        if (edge.w < neighbor_entry_a.w) {
          neighbor_entry_a.v = vertex;
          neighbor_entry_a.u = edge.end;
          neighbor_entry_a.w = edge.w;
          neighbor_entry_b = neighbor_entry_a;
        }
      }
    }
    return map;
  }

  auto join_clusters_odd(const Graph& remaining, Graph& spanner,
      const unordered_set<int>& remaining_vertices,
      const std::unordered_map<int, int>& clustering) {
    auto cluster_pairs_map = cluster_pair_maps(remaining, remaining_vertices,
        clustering, clustering);
    for(const auto& c1 : cluster_pairs_map) {
       for (const auto& c2 : c1.second) {
          spanner.add_edge(c2.second.u, c2.second.v, c2.second.w);
       }
    }
  }

  auto join_clusters_even(const Graph& remaining, Graph& spanner,
      const std::unordered_set<int>& remaining_vertices,
      const std::unordered_map<int, int>& last_clustering,
      const std::unordered_map<int, int>& before_last_clustering) {

    auto cluster_pairs_map = cluster_pair_maps(remaining, remaining_vertices,
        last_clustering, before_last_clustering);
    for(const auto& c1 : cluster_pairs_map) {
       for (const auto& c2 : c1.second) {
          spanner.add_edge(c2.second.u, c2.second.v, c2.second.w);
       }
    }
  }
} // namespace



Graph two_k_minus_1_spanner(int k, Graph g, std::ostream& out) {
  Graph spanner(g.size());
  bool use_rewrite = util::get_bool_flag("use_new_alg");
  if (use_rewrite) {
    auto end_of_phase_1 = form_clusters_2(g, spanner, k, k/2);
    if (k % 2 == 0) {
      join_clusters_even(end_of_phase_1.not_added, spanner,
          end_of_phase_1.remaining_vertices,
          end_of_phase_1.last_clustering,
          end_of_phase_1.before_last);
    } else {  // k is odd
      join_clusters_odd(end_of_phase_1.not_added, spanner,
          end_of_phase_1.remaining_vertices,
          end_of_phase_1.last_clustering);
    }

  } else {
    auto end_of_phase_1 = form_clusters(g, spanner, k, out);
    join_clusters(end_of_phase_1.not_added, spanner,
        end_of_phase_1.last_clustering,
        ((k % 2) == 0) ? end_of_phase_1.before_last :
        end_of_phase_1.last_clustering);
  }

  return spanner;
}

namespace {
  void join_clusters_2(Graph not_added, Graph& spanner,
      const std::unordered_set<int>& remaining_vertices,
      const std::unordered_map<int, int>& clustering) {
    for (auto v : remaining_vertices) {
      auto min_edges = cluster_to_min_edge_map(not_added, v, clustering);
      not_added.clear_neighbors(v);
      for (auto&& cluster_edge : min_edges) {
        spanner.add_edge(v, cluster_edge.second.end, cluster_edge.second.w);
      }
    }
  }
}  // namespace

Graph two_k_minus_1_spannerv2(int k, Graph g) {
   Graph spanner(g.size()); 
   auto end_of_phase_1 = form_clusters_2(g, spanner, k, k-1);
   join_clusters_2(end_of_phase_1.not_added, spanner,
       end_of_phase_1.remaining_vertices, end_of_phase_1.last_clustering);
   return spanner;
}
    

}  // namespace graphs
