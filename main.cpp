#include "sample.cpp"
#include <cassert>
#include <iostream>
#include <fstream>
#include "visualizer.h"

using namespace antlogic;

vector<vector<ll> > meals;

void load_map(const string &fn) {
  ifstream in(fn.c_str());
  ll w, h;
  in >> w >> h;
  meals.resize(h, vector<ll>(w));
  for (ll i = 0; i < h; ++i) {
    for (ll j = 0; j < w; ++j) {
      in >> meals[i][j];
    }
  }
  in.close();
}

char mem[MAX_MEMORY*10];
struct MyAnt : public Ant {
  bool has_food;
  ll x, y, id;

  MyAnt() {
    ;//    fill(mem+MAX_MEMORY*id, mem+MAX_MEMORY*id+MAX_MEMORY, 0);
    has_food = false;
    x = 0;
    y = 0;
  }
  virtual char *getMemory() const {
    return mem+MAX_MEMORY*id;
  }
  virtual bool hasFood() const {
    return has_food;
  }
  virtual int getTeamId() const {
    return 0;
  }

  void next_step() {
    AntSensor sensors[3][3];
    for (ll i = 0; i < 3; ++i) {
      for (ll j = 0; j < 3; ++j) {
	if (x+i-1 < 0 || y+j-1 < 0) {
	  sensors[i][j].isWall = true;
	  continue;
	}
	if (x+i-1 >= meals[0].size() || y+j-1 >= meals.size()) {
	  sensors[i][j].isWall = true;
	  continue;
	}
	if (meals[y+j-1][x+i-1] > 0) {
	  sensors[i][j].isFood = true;
	}
	if (x+i-1==0 && y+j-1==0) {
	  sensors[i][j].isMyHill = true;
	}
      }
    }
    
    AntAction action;
    action = GetAction(*this, sensors);

    if (action.actionType == MOVE_UP) {
      if (y > 0) {
	--y;
      }
    } else if (action.actionType == MOVE_LEFT) {
      if (x > 0) {
	--x;
      }
    } else if (action.actionType == MOVE_DOWN) {
      if (y+1 < meals.size()) {
	++y;
      }
    } else if (action.actionType == MOVE_RIGHT) {
      if (x+1 < meals.size()) {
	++x;
      }
    } else if (action.actionType == GET) {
      if (meals[y][x] > 0 && !has_food) {
	--meals[y][x];
	has_food = true;
      }
    } else if (action.actionType == PUT && has_food) {
      ++meals[y][x];
      has_food = false;
    }
  }
};

int main() {
  MyAnt ant[10];
  for (ll i = 0; i < 10; ++i) {
    ant[i].id = i;
  }
  fill(mem, mem+MAX_MEMORY*10, 0);

  load_map("/home/r00tman/olymp/losh/2014/ants/simpla.map");

  visualizer viz;
  
  std::thread t = thread([&](){
      viz.init();
      viz.run();
      viz.destroy();
    });

  drawer dr;
  dr.meals = &meals;
  dr.add_grid(&viz, 10);

  for (ll turn = 0; turn < 1000; ++turn) {
    cout << turn << " " << ant[0].x << " " << ant[0].y << " " << meals[0][0] << endl;
    
    for (ll i = 0; i < 10; ++i) {
      ant[i].next_step();
      dr.update_ant(i, ant[i].x, ant[i].y, ant[i].has_food);
    }
    //    ant2.next_step();
    
    dr.update_meals();
    usleep(50000);
  }

  t.join();
  
  return 0;
}
