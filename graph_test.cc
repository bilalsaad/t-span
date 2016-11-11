// TODO this file
#include "three-spanner-algorithm.h"
#include <iostream>
using namespace std;
using namespace graphs;

int main() {
  auto g = randomGraph(4);
  cout << "input graph : ";
  cout << g;
  std::cout << three_spanner(g).edges();
  return 0;
 }
