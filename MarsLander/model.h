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

  std::vector<double> values;

  vertex_array_object* _vao;
  buffer_object *_vbo_array;
  };


void fill_render_data(model& m, const std::vector<double>& values);

