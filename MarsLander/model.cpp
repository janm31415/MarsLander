#include "model.h"
#include "logging.h"

#include "buffer_object.h"
#include "vertex_array_object.h"

#include <algorithm>
#include <sstream>
#include <numeric>

namespace
  {
  void gl_check_error(const char *txt)
    {
    unsigned int err = glGetError();
    if (err)
      {
      std::stringstream str;
      str << "GL error " << err << ": " << txt;
      throw std::runtime_error(str.str());
      }
    }
  }


model::model() : _vao(nullptr), _vbo_array(nullptr)
  {

  }

model::~model()
  {
  delete_render_objects();
  }

void model::delete_render_objects()
  {
  if (_vao)
    {
    _vao->release();
    delete _vao;
    _vao = nullptr;
    }
  if (_vbo_array)
    {
    _vbo_array->release();
    delete _vbo_array;
    _vbo_array = nullptr;
    }
  }

void fill_render_data(model& m, const std::vector<double>& values)
  {
  m.delete_render_objects();
  double cutoff = 1000000.0;
  double max_value = *std::max_element(m.values.begin(), m.values.end());
  double min_value = *std::min_element(m.values.begin(), m.values.end());
  if (max_value > cutoff)
    max_value = cutoff;
  if (min_value < -cutoff)
    min_value = -cutoff;
  if (min_value == max_value)
    {
    min_value -= 1e-4;
    max_value += 1e-4;
    }
  double range_original = max_value - min_value;

  double max_value2 = *std::max_element(values.begin(), values.end());
  double min_value2 = *std::min_element(values.begin(), values.end());

  double range2 = max_value2 - min_value2;

  if (range2 <= range_original)
    {
    double diff = range_original - range2;
    min_value = min_value2 - diff / 2.0;
    max_value = max_value2 + diff / 2.0;
    }
  else
    {
    double diff = range2 - range_original;
    min_value = min_value2 + diff / 2.0;
    max_value = max_value2 - diff / 2.0;
    }


  std::vector<GLfloat> vertices;
  vertices.reserve(values.size() * 2);
  uint64_t n = values.size();
  for (uint64_t i = 0; i < n; ++i)
    {
    float pos_x = (float)i / (float)(n - 1) * 2.f - 1.f;
    float pos_y = ((float)values[i] - (float)min_value) / ((float)max_value - (float)min_value) * 1.8f - 0.9f;
    vertices.push_back(pos_x);
    vertices.push_back(pos_y);
    }

  m._vao = new vertex_array_object();
  m._vao->create();
  gl_check_error(" _vao->create()");
  m._vao->bind();
  gl_check_error(" _vao->bind()");

  m._vbo_array = new buffer_object(GL_ARRAY_BUFFER);
  m._vbo_array->create();
  gl_check_error("_vbo_array->create()");
  m._vbo_array->bind();
  gl_check_error("_vbo_array->bind()");
  m._vbo_array->set_usage_pattern(GL_STATIC_DRAW);
  m._vbo_array->allocate(vertices.data(), (int)(sizeof(GLfloat) * vertices.size()));
  gl_check_error("_vbo_array->allocate()");
  
  m._vao->release();
  gl_check_error("m._vao->release()");
  m._vbo_array->release();
  gl_check_error("m._vbo_array->release()");
  }
