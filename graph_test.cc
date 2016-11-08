// TODO this file
#include "graph.h"
using namespace std;
using namespace graphs;

int main() {
  auto g = randomGraph(4000);
  cout << "g has yay many edges " << g.edges();
  return 0;
 }
