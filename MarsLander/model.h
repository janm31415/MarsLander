#pragma once

#include <vector>
#include <stdint.h>
#include <string>

#include "cgalgo.h"

namespace jtk
  {
  class buffer_object;
  class vertex_array_object;
  }


struct model
  {
  model();
  ~model();

  void delete_render_objects();
  
  int number_of_terrain_points;
  
  population current_population, next_population;
  std::vector<double> current_population_normalized_score;

  jtk::vertex_array_object* _vao;
  jtk::buffer_object *_vbo_array;
  
  std::vector<jtk::vertex_array_object*> _path_vao;
  std::vector<jtk::buffer_object*> _path_vbo_array;
  };


void init_model(model& m, const std::string& s);

void make_random_population(model& m);

void fill_renderer_with_simulation(model& m);

void make_next_generation(model& m);

void fill_terrain_data(model& m);

void simulate_population(model& m);

void get_best_run_results(simulation_data& sd, simulation_data& prev_sd, const model& m);

