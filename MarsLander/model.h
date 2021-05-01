#pragma once

#include <vector>
#include <stdint.h>
#include <string>

#include "cgalgo.h"

class buffer_object;
class vertex_array_object;


struct model
  {
  model();
  ~model();

  void delete_render_objects();
  
  int number_of_terrain_points;
  
  population current_population;
  std::vector<double> current_population_normalized_score;

  vertex_array_object* _vao;
  buffer_object *_vbo_array;
  
  std::vector<vertex_array_object*> _path_vao;
  std::vector<buffer_object*> _path_vbo_array;
  };


void init_model(model& m, const std::string& s);

void make_random_population(model& m);

void make_next_generation(model& m);

void fill_terrain_data(model& m);

void simulate_population(model& m);

