#include "three-spanner-algorithm.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <ctime>
#include <iostream>
#include <cstdlib>

namespace graphs {
  
namespace {
  using namespace std;
  using Clusters = std::unordered_map<int, std::unordered_set<int>>; 
  auto sample(const Graph& g) {
    Clusters sampled_vertices;
    vector<int> non_sampled_vertices;
    srand(std::time(0));
    for (int i = 0; i < g.size(); ++i) {
      if ((std::rand() % 100) < 50) { 
        sampled_vertices[i] = {};
        cout << "SAMPLE: " << i << endl;
      } else {
        non_sampled_vertices.push_back(i);
      }
    }
    return std::make_pair(sampled_vertices, non_sampled_vertices);
  } 

  auto form_clusters(const Graph& g) {
    auto samples = sample(g);
    auto& clusters = samples.first;
    const auto& unsampled_vertices = samples.second;
    Graph spanner(g.size());
    Graph not_added(g.size());

    for(auto&& unsampled_vertex : unsampled_vertices) {
      const auto& neighbors = g.Neighbors(unsampled_vertex);
      Edge sentinel_edge{-1, 0}; 
      // Pick the best edge adjacent to a sampled vertex, if there are non the
      // sentinel edge is returned.
      auto best_edge = accumulate(begin(neighbors), end(neighbors),
          sentinel_edge,
          [&clusters] (Edge acc, auto&& next_edge) {
              if (clusters.count(next_edge.end) != 0) {
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
        clusters[best_edge.end].insert(unsampled_vertex);
        spanner.add_edge(unsampled_vertex, best_edge.end, best_edge.w);
        for (auto&& edge : neighbors) {
          if (edge < best_edge)
            spanner.add_edge(unsampled_vertex, edge.end, edge.w);
          // What happens if there is an equal edge?
          else if (edge > best_edge) {
            not_added.add_edge(unsampled_vertex, edge.end, edge.w);
          }
        }
      }
    }

    Graph graph_without_intra_cluster_edges =
      filter_edges(not_added, [&clusters] (int start, int end) {
          return 
          (clusters.count(start) != 0 && clusters[start].count(end) != 0) ||
          (clusters.count(end) != 0 && clusters[end].count(start) != 0);
          }); 

    cout << "spanner: " << spanner;
    cout << "not_added " << graph_without_intra_cluster_edges;
    // We could remove intra-edge clusters from not_added.
    return make_pair(spanner, graph_without_intra_cluster_edges);
  }
  
  auto join_clusters(Clusters& clusters, const Graph& not_added_graph) {

  }
}  // namespace.
  // This is an implementaiton of <TODO algorithm name here for 3 span case>
  // which computes a 3-spanner of 'g'.
  // 
  // The algorithm selects edges to be included in the spanner in two phases.
  // Step 1. Forming the clusters:
  //  * Choose a random sample R from V(g), by picking each vertex indepently
  //    with probability n^(-1/2).
  //  * Form clusters (of vertices) around the sampled vertices. These clusters
  //    should start with only the vertices from R. These will be the centers of
  //    each cluster.
  //  * For each unsampled v in V(g) - R: 
  //      - If v is not adjacent to any sampled vertex, add all edges incident
  //        on v to the spanner.
  //      - If v is adjacent to one or more sampled vertices, let N(v,R) be the
  //        sampled vertex that is nearest to v: add (v, N(v,R) and all edges
  //        incident on v that weigh less than (v, N(v,R)). v is added to the
  //        cluster centered at N(v, R).
  // At the end of the first phase, Let E' denote the set of all edges of the
  // given graph excluding the edges added to the spanner and all the
  // intra-cluster edges. Let V' denote the set of vertices corresponding to the
  // end points of E'. It should be noted that each vertex of V' is either a
  // sampled vertex or is adjacent to some sampled vertex, and the first phase
  // has partitioned V' into disjoint clusters each centered around some sample
  // vertex. The graph G(V', E') is passed onto the second phase.
  //
  // Step 2. Joining the Clusters. 
  //  * For each vertex v in V', group all its neighbors into their respective
  //    clusters. There will be at most |R| neighboring clusters of v. For each
  //    cluster adjacent to v, we select the least weight edge among all the
  //    edges between (the vertices of) the cluster and v, and add it to the
  //    spanner.
  //
  // Let E<s,1> and E<s,2> be the sets of edges added to the spanner in the
  // first phase and the second phase respectively. G(V, E1 union E2) is a
  // spanner.
  Graph three_spanner(const Graph& g) {
    return form_clusters(g).second;   
  }


} // namespace graphs
