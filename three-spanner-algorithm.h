#ifndef THREESPANNER_H
#define THREESPANNER_H
#include "graph.h"
namespace graphs {

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
  Graph three_spanner(Graph g);
} // namespace graphs



#endif
