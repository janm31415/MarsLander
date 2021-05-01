#pragma once

#include <vector>
#include <stdint.h>
#include <string>

class buffer_object;
class vertex_array_object;


struct model
  {
  model();
  ~model();

  void delete_render_objects();
  
  int number_of_terrain_points;

  vertex_array_object* _vao;
  buffer_object *_vbo_array;
  };


void init_model(model& m, const std::string& s);

void fill_terrain_data(model& m);

