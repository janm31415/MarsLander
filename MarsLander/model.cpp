
#include "model.h"
#include "logging.h"
#include "cgalgo.h"

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
  
void init_model(model& m, const std::string& s) {
  std::stringstream ss;
  ss << s;
  std::stringstream logss;
  read_input(ss, logss);
  Logging::Info() << logss.str();
}

void fill_terrain_data(model& m)
  {
  m.delete_render_objects();
  m.number_of_terrain_points = surface_points.size();
  std::vector<GLfloat> vertices;
  vertices.reserve(m.number_of_terrain_points * 2);
  uint64_t n = m.number_of_terrain_points;
  for (uint64_t i = 0; i < n; ++i)
    {
    vertices.push_back(surface_points[i].x/(float)W*2.f-1.f);
    vertices.push_back(surface_points[i].y/(float)H*2.f-1.f);
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
