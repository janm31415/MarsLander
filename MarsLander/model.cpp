
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
  for (size_t i = 0; i < _path_vao.size(); ++i) {
    if (_path_vao[i]) {
      _path_vao[i]->release();
      delete _path_vao[i];
      _path_vao[i] = nullptr;
    }
    if (_path_vbo_array[i]) {
      _path_vbo_array[i]->release();
      delete _path_vbo_array[i];
      _path_vbo_array[i] = nullptr;
    }
  }
}

void init_model(model& m, const std::string& s) {
  std::stringstream ss;
  ss << s;
  std::stringstream logss;
  read_input(ss, logss);
  Logging::Info() << logss.str();
}

void make_random_population(model& m) {
  m.current_population = generate_random_population(simdata.R, simdata.P);
}

void make_next_generation(model& m) {
  m.current_population = make_next_generation(m.current_population, m.current_population_normalized_score);
}

void fill_terrain_data(model& m)
{
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

void fill_renderer_with_simulation(model& m) {
  m._path_vao.clear();
  m._path_vbo_array.clear();
  for (int i = 0; i < m.current_population.size(); ++i) {
    std::vector<vec2<float>> path;
    evaluate(path, m.current_population[i]);
    std::vector<GLfloat> vertices;
    vertices.reserve(path.size() * 2);
    for (uint64_t i = 0; i < path.size(); ++i)
    {
      vertices.push_back(path[i].x/(float)W*2.f-1.f);
      vertices.push_back(path[i].y/(float)H*2.f-1.f);
    }
    m._path_vao.push_back(new vertex_array_object());
    m._path_vao.back()->create();
    gl_check_error(" _path_vao.back()->create()");
    m._path_vao.back()->bind();
    gl_check_error(" _path_vao.back()->bind()");
  
    m._path_vbo_array.push_back(new buffer_object(GL_ARRAY_BUFFER));
    m._path_vbo_array.back()->create();
    gl_check_error("_path_vbo_array.back()->create()");
    m._path_vbo_array.back()->bind();
    gl_check_error("_path_vbo_array.back()->bind()");
    m._path_vbo_array.back()->set_usage_pattern(GL_STATIC_DRAW);
    m._path_vbo_array.back()->allocate(vertices.data(), (int)(sizeof(GLfloat) * vertices.size()));
    gl_check_error("_path_vbo_array.back()->allocate()");
    
    m._path_vao.back()->release();
    gl_check_error("m._path_vao.back()->release()");
    m._path_vbo_array.back()->release();
    gl_check_error("m._path_vbo_array.back()->release()");
  }
  //for (auto s : m.current_population_normalized_score) {
  //  Logging::Info() << "score " << s << "\n";
  //}
}

void simulate_population(model& m) {
  std::vector<int64_t> scores;
  scores.reserve(m.current_population.size());
  for (int i = 0; i < m.current_population.size(); ++i) {
    std::vector<vec2<float>> path;
    scores.push_back(evaluate(path, m.current_population[i]));
  }
  m.current_population_normalized_score = normalize_scores_roulette_wheel(scores);
}

simulation_data get_best_run_results(const model& m) {
  int besti = 0;
  double score = 0.0;
  for (int i = 0; i < m.current_population_normalized_score.size(); ++i) {
    if (m.current_population_normalized_score[i]>score) {
      score = m.current_population_normalized_score[i];
      besti = i;
    }
  }
  return run_chromosome(m.current_population[besti]);
}
