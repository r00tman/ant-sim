#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <GLFW/glfw3.h>
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

struct point {
  float x, y;

  point() {
  }

  point(float _x, float _y) {
    x = _x;
    y = _y;
  }
};

static void glVertex3f(point p) {
  glVertex3f(p.x, p.y, 0);
}

struct visualizer {
  GLFWwindow* window;
  const double scaled = 1/50.0;

  static void error_callback(int error, const char* description) {
    fputs(description, stderr);
  }
  
  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GL_TRUE);
      exit(0);
    }
  }

  void init() {
    glfwSetErrorCallback(visualizer::error_callback);
    if (!glfwInit()) {
      exit(EXIT_FAILURE);
    }
    window = glfwCreateWindow(1366, 768, "Simple example", glfwGetPrimaryMonitor(), NULL);
    if (!window) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glfwSwapInterval(1);
    
    glLineWidth(3);
    
    glfwSetKeyCallback(window, visualizer::key_callback);
  }

  struct shape {
    float r, g, b;

    virtual void draw(std::function<point(point)> ft) {
    }
  };

  struct rect_shape : public shape {
    float x, y, w, h;
    
    virtual void draw(std::function<point(point)> ft) {
      glColor3f(r, g, b);
      glBegin(GL_TRIANGLES);

      glVertex3f(ft(point(x, y)));
      glVertex3f(ft(point(x+w, y)));
      glVertex3f(ft(point(x+w, y+h)));
      glVertex3f(ft(point(x+w, y+h)));
      glVertex3f(ft(point(x, y+h)));
      glVertex3f(ft(point(x, y)));

      glEnd();
    }
  };

  struct line_shape : public shape {
    point p1, p2;
    float width;
    
    virtual void draw(std::function<point(point)> ft) {
      glLineWidth(width);
      glColor3f(r, g, b);
      glBegin(GL_LINES);
      
      glVertex3f(ft(p1));
      glVertex3f(ft(p2));

      glEnd();
    }
  };

  std::vector<shape*> m_shapes;

  void run() {
    while (!glfwWindowShouldClose(window)) {
      float ratio;
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      ratio = width / (float) height;
      glViewport(0, 0, width, height);
      glClear(GL_COLOR_BUFFER_BIT);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glColor3f(1.f, 1.f, 0.f);

      for (size_t i = 0; i < m_shapes.size(); ++i) {
	m_shapes[i]->draw([&](point p) {
	    return point(p.x*scaled-1, 1-p.y*scaled);
	  });
      }
    
      glfwSwapBuffers(window);
      glfwPollEvents();
    }
  }
  
  void destroy() {
    glfwDestroyWindow(window);
    glfwTerminate();
  }
};

struct drawer {
  std::vector<std::vector<long long> > *meals;
  std::vector<std::vector<visualizer::rect_shape*> > cells;
  std::vector<std::vector<bool> > marked;

  std::vector<visualizer::rect_shape*> ant_shapes;
  const float CSIZE = 3;
  const float MEAL_COLOR = 0.8;
  
  void add_grid(visualizer *viz, int ant_count) {
    cells.resize(meals->size(), std::vector<visualizer::rect_shape*>(meals->begin()->size()));
    marked.resize(cells.size(), std::vector<bool>(cells[0].size()));

    for (int i = 0; i < meals->size(); ++i) {
      for (int j = 0; j < meals->begin()->size(); ++j) {
	visualizer::rect_shape *cell = new visualizer::rect_shape();
	cell->x = j*CSIZE;
	cell->y = i*CSIZE;
	cell->w = cell->h = CSIZE;
	viz->m_shapes.push_back(cell);
	cells[i][j] = cell;
      }
    }
    update_meals();
    for (int i = 0; i < meals->size(); ++i) {
      visualizer::line_shape *line = new visualizer::line_shape();
      line->p1 = point(0, i*CSIZE);
      line->p2 = point(meals->begin()->size()*CSIZE, i*CSIZE);
      line->width = 1;
      
      line->r = line->g = line->b = 0.2;
      viz->m_shapes.push_back(line);
    }

    for (int i = 0; i < meals->begin()->size(); ++i) {
      visualizer::line_shape *line = new visualizer::line_shape();
      line->p1 = point(i*CSIZE, 0);
      line->p2 = point(i*CSIZE, meals->size()*CSIZE);
      line->width = 2;
      
      line->r = line->g = line->b = 0.2;
      viz->m_shapes.push_back(line);
    }
    for (int i = 0; i < ant_count; ++i) {
      visualizer::rect_shape *ant_shape;
      ant_shape = new visualizer::rect_shape();
      ant_shape->x = 0;
      ant_shape->y = 0;
      ant_shape->w = ant_shape->h = CSIZE;
      ant_shape->r = ant_shape->g = ant_shape->b = 0.2;
      ant_shape->b = 0.5f;

      ant_shapes.push_back(ant_shape);
      viz->m_shapes.push_back(ant_shape);
      
    }
  }

  void update_ant(int i, int x, int y, bool hasFood) {
    ant_shapes[i]->x = CSIZE*x;
    ant_shapes[i]->y = CSIZE*y;
    if (hasFood) {
      ant_shapes[i]->g = 0.5;
    } else {
      ant_shapes[i]->g = 0.2;
    }
  }

  void update_meals() {
    for (int i = 0; i < cells.size(); ++i) {
      for (int j = 0; j < cells[0].size(); ++j) {
	float val = pow(MEAL_COLOR, (*meals)[i][j]);
	cells[i][j]->r = cells[i][j]->g = cells[i][j]->b = val;
	if (marked[i][j]) {
	  cells[i][j]->b = 0.9;
	}
      }
    }
  }

  void mark_cell(int x, int y, bool m) {
    marked[y][x] = m;
  }
};

#endif // VISUALIZER_H
















