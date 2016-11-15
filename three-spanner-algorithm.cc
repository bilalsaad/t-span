#include "three-spanner-algorithm.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <ctime>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include "util.h"

namespace graphs {
  using scoped_timer = util::scoped_timer;
namespace {
  using namespace std;
  using Clusters = vector<int>; 
  auto sample(const Graph& g) {
    scoped_timer st("sample"); 
    Clusters sampled_vertices(g.size(), -1);
    vector<int> non_sampled_vertices;
    srand(std::time(0));
    for (int i = 0; i < g.size(); ++i) {
     if ((std::rand() % 100) < 30) { 
        sampled_vertices[i] = i;
      } else {
        non_sampled_vertices.push_back(i);
      }
    }
    return std::make_pair(sampled_vertices, non_sampled_vertices);
  } 

  auto form_clusters(const Graph& g, Graph& spanner) {
    scoped_timer st("form_cluster");
    auto samples = sample(g);
    auto& clusters = samples.first;
    const auto& unsampled_vertices = samples.second;

    // Possible optimization - give up on the unsampled_vertices and only use
    // the clusters.
    for(auto&& unsampled_vertex : unsampled_vertices) {
      const auto& neighbors = g.neighbors(unsampled_vertex);
      Edge sentinel_edge{-1, 0}; 
      // Pick the best edge adjacent to a sampled vertex, if there are non the
      // sentinel edge is returned.
      auto best_edge = accumulate(begin(neighbors), end(neighbors),
          sentinel_edge,
          [&clusters] (Edge acc, auto&& next_edge) {
              if (clusters[next_edge.end] == next_edge.end) {
                return (acc.end == -1) ? next_edge :
                       (next_edge.w < acc.w ? next_edge : acc);
              } else {
                return acc;
              }
          });
      // This vertex is not adjacent to any sampled vertices - add all of its
      // edges to the spanner.
      if (best_edge == sentinel_edge) {
        spanner.add_vertex_with_edges(unsampled_vertex, neighbors); 
      } else {
        // Add this vertex to the cluster at best_edge.end
        clusters[unsampled_vertex] = best_edge.end;
        spanner.add_edge(unsampled_vertex, best_edge.end, best_edge.w);
        for (auto&& edge : neighbors) {
          if (edge < best_edge)
            spanner.add_edge(unsampled_vertex, edge.end, edge.w);
        }
      }
    }
    Graph graph_without_intra_cluster_edges =
      filter_edges(g, [&clusters, &spanner] (int start, int end) {
          // Remove edges that are in spanner and intra cluster edges. 
          return spanner.has_edge(start, end) ||
         (clusters[start] != -1 && clusters[start] == clusters[end]);
          }); 
    return make_pair(std::move(clusters),
        std::move(graph_without_intra_cluster_edges));
  }
  
  auto join_clusters(const Clusters& clusters, const Graph& not_added_graph,
                     Graph& spanner) {
    scoped_timer st("join_clusters");
    for (int i = 0; i < not_added_graph.size() ; ++i) {
      const auto& neighbors = not_added_graph.neighbors(i);
      unordered_map<int, Edge> cluster_representives;
      for (const auto& e : neighbors) {
        assert(clusters[e.end] != -1);
        auto current_rep = cluster_representives.find(e.end);
        if (current_rep == std::end(cluster_representives)) { 
          cluster_representives.emplace(clusters[e.end], e);
        } else if (current_rep->second > e) {
          current_rep->second = e;
         }
      }
      for (auto&& reps : cluster_representives) {
        spanner.add_edge(i, reps.second.end, reps.second.w);
      }
    } 
  }
}  // namespace.

Graph three_spanner(const Graph& g) {
  scoped_timer st("three-span");
  Graph spanner(g.size());
  auto clusters_and_not_added_edges = form_clusters(g, spanner);
  join_clusters(clusters_and_not_added_edges.first,
      clusters_and_not_added_edges.second,
      spanner);
  return spanner;
}
} // namespace graphs
