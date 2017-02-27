#ifndef TWO_K_MINUS_1_H
#define TWO_K_MINUS_1_H
#include "graph.h"

namespace graphs {

  // This is the algorithm described in the article.
  Graph two_k_minus_1_spanner(int k, Graph g, std::ostream& out = std::cout);
  // This is the algorithm described in
  // https://u.cs.biu.ac.il/~liamr/spanner.pdf
  // Which does k-1 iterations of the firt phase and a simpler joining of
  // clusters.
  Graph two_k_minus_1_spannerv2(int k, Graph g);

}  // namespace graphs.
#endif
