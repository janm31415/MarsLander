#include "shader.h"
#include "logging.h"

static const char* types[] = {
    "Vertex",
    "Fragment",
    ""
  };

shader::shader(shader::shader_type shader_type)
  : _shader_id(0),
  _shader_type(shader_type)
  {
  }

shader::~shader()
  {
  }

bool shader::compile_source_code(const char* source)
  {
  return compile(source);
  }

bool shader::compile_source_code(const std::string& source)
  {
  return compile(source.c_str());
  }

bool shader::create()
  {
  if (_shader_type == shader::shader_type::Vertex)
    _shader_id = glCreateShader(GL_VERTEX_SHADER);
  else if (_shader_type == shader::shader_type::Fragment)
    _shader_id = glCreateShader(GL_FRAGMENT_SHADER);

  if (!_shader_id)
    {
    if (_shader_type == shader::shader_type::Vertex)
      Logging::Warning() << "Could not create shader of type Vertex\n";
    if (_shader_type == shader::shader_type::Fragment)
      Logging::Warning() << "Could not create shader of type Fragment\n";
    else
      Logging::Warning() << "Could not create shader\n";
    return false;
    }
  else
    return true;
  }

void shader::destroy()
  {
  if (!_shader_id)
    return;

  glDeleteShader(_shader_id);
  _shader_id = 0;
  }

bool shader::compile(const char* source)
  {
  if (!create())
    return false;

  glShaderSource(_shader_id, 1, &source, nullptr);
  glCompileShader(_shader_id);

  int value;
  glGetShaderiv(_shader_id, GL_COMPILE_STATUS, &value);
  _compiled = (value != 0);

  int source_codeLength = 0;
  glGetShaderiv(_shader_id, GL_SHADER_SOURCE_LENGTH, &source_codeLength);
  if (source_codeLength > 1)
    {
    int temp = 0;
    _source_code.resize(source_codeLength);
    glGetShaderSource(_shader_id, source_codeLength, &temp, &_source_code[0]);
    }

  if (!_compiled)
    {
    glGetShaderiv(_shader_id, GL_INFO_LOG_LENGTH, &value);

    if (value > 1)
      {
      int length;
      _log.resize(value);
      glGetShaderInfoLog(_shader_id, value, &length, &_log[0]);

      const char* type = types[2];
      if (_shader_type == shader::shader_type::Vertex)
        type = types[0];
      else if (_shader_type == shader::shader_type::Fragment)
        type = types[1];
      Logging::GetInstance() << _log.c_str() << "\n";
      }
    }

  return _compiled;
  }
