// TODO this file
#include "three-spanner-algorithm.h"
#include <iostream>
#include <string>
using namespace std;
using namespace graphs;

int main(int argc, char** argv) {
  auto g = randomGraph(argc > 1 ? std::stoi(*(argv+1)) : 5);
  cout << "input graph:\n" << g;
  auto threespan = three_spanner(g);
  cout << "three spanner:\n" << threespan;
  return 0;
 }
