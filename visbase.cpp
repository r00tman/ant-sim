#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <thread>
#include <functional>
#include <unistd.h>
#include "visualizer.h"

using namespace std;

int main(int argc, char *argv[]) {
  visualizer viz;
  
  std::thread t = thread([&](){
      viz.init();
      viz.run();
      viz.destroy();
    });
  drawer dr;
  dr.meals = new vector<vector<long long> >(10, vector<long long>(10, 0));
  dr.add_grid(&viz);

  t.join();

  return 0;
}
