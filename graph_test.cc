// TODO this file
#include "graph.h"
using namespace std;
using namespace graphs;

int main() {
  auto g = randomGraph(400);
  cout << "g has yay many edges " << g.edges() << "\n";
  return 0;
 }
