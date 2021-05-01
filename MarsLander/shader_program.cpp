
#include "shader_program.h"

#include "logging.h"

shader_program::shader_program()
  : _program_id(0),
  _vertex_shader(nullptr),
  _fragment_shader(nullptr)
  {
  }

shader_program::~shader_program()
  {
  remove_all_shaders();
  }

bool shader_program::add_shader(shader *shader)
  {
  if (shader->shadertype() == shader::shader_type::Vertex)
    {
    if (_vertex_shader != nullptr)
      return false;
    _vertex_shader = shader;
    }
  else if (shader->shadertype() == shader::shader_type::Fragment)
    {
    if (_fragment_shader != nullptr)
      return false;
    _fragment_shader = shader;
    }

  return true;
  }

bool shader_program::add_shader_from_source(shader::shader_type type, const char* source)
  {
  shader *s = new shader(type);

  if (s->compile_source_code(source))
    {
    if (s->is_compiled())
      {
      if (add_shader(s))
        return true;
      else
        {
        s->destroy();
        delete s;
        return false;
        }
      }
    else
      {
      Logging::Warning() << "Compile shader error: " << s->log().c_str() << "\n";
      s->destroy();
      delete s;
      return false;
      }
    }
  s->destroy();
  delete s;
  return false;
  }

bool shader_program::add_shader_from_source(shader::shader_type type, const std::string& source)
  {
  return add_shader_from_source(type, source.c_str());
  }

void shader_program::remove_all_shaders()
  {
  if (_program_id && _vertex_shader && _fragment_shader)
    {
    glDetachShader(_program_id, _vertex_shader->shader_id());
    _vertex_shader->destroy();
    delete _vertex_shader;
    _vertex_shader = nullptr;

    glDetachShader(_program_id, _fragment_shader->shader_id());
    _fragment_shader->destroy();
    delete _fragment_shader;
    _fragment_shader = nullptr;

    _linked = false;
    }
  }

bool shader_program::create()
  {
  _program_id = glCreateProgram();

  if (!_program_id)
    {
    Logging::Warning() << "Could not create program object\n";
    return false;
    }

  return true;
  }

bool shader_program::link()
  {
  _linked = false;

  if (!create())
    return false;

  if (!_vertex_shader || !_fragment_shader)
    return false;

  glAttachShader(_program_id, _vertex_shader->shader_id());
  glAttachShader(_program_id, _fragment_shader->shader_id());
  glLinkProgram(_program_id);

  int value = 0;
  glGetProgramiv(_program_id, GL_LINK_STATUS, &value);
  _linked = (value != 0);

  if (!_linked)
    {
    int length = 0;
    glGetProgramiv(_program_id, GL_INFO_LOG_LENGTH, &value);
    if (value > 1)
      {
      _log.resize(value);
      glGetProgramInfoLog(_program_id, value, &length, &_log[0]);
      Logging::Warning() << "shader program: link error: " << _log.c_str() << "\n";
      }

    remove_all_shaders();
    }

  return _linked;
  }

bool shader_program::bind()
  {
  if (!_program_id || !_linked)
    return false;

  glUseProgram(_program_id);
  return true;
  }

void shader_program::release()
  {
  glUseProgram(0);
  }

GLint shader_program::attribute_location(const char* name)
  {
  return glGetAttribLocation(_program_id, name);;
  }

GLint shader_program::attribute_location(const std::string& name)
  {
  return glGetAttribLocation(_program_id, name.c_str());
  }

void shader_program::bind_attribute_location(const char* name, int location)
  {
  glBindAttribLocation(_program_id, location, name);
  }

void shader_program::bind_attribute_location(const std::string& name, int location)
  {
  glBindAttribLocation(_program_id, location, name.c_str());
  }

void shader_program::disable_attribute_array(int location)
  {
  glDisableVertexAttribArray(location);
  }

void shader_program::disable_attribute_array(const char* name)
  {
  int location = glGetAttribLocation(_program_id, name);
  glDisableVertexAttribArray(location);
  }

void shader_program::disable_attribute_array(const std::string& name)
  {
  int location = glGetAttribLocation(_program_id, name.c_str());
  glDisableVertexAttribArray(location);
  }

void shader_program::enable_attribute_array(int location)
  {
  glEnableVertexAttribArray(location);
  }

void shader_program::enable_attribute_array(const char* name)
  {
  int location = glGetAttribLocation(_program_id, name);
  glEnableVertexAttribArray(location);
  }

void shader_program::enable_attribute_array(const std::string& name)
  {
  int location = glGetAttribLocation(_program_id, name.c_str());
  glEnableVertexAttribArray(location);
  }

void shader_program::set_attribute_array(int location, const GLfloat* values, int tupleSize, int stride)
  {
  glVertexAttribPointer(location, tupleSize, GL_FLOAT, GL_FALSE, stride, values);
  }

void shader_program::set_attribute_array(int location, GLenum type, const void* values, int tupleSize, int stride)
  {
  glVertexAttribPointer(location, tupleSize, type, GL_TRUE, stride, values);
  }

void shader_program::set_attribute_buffer(int location, GLenum type, int offset, int tupleSize, int stride)
  {
  glVertexAttribPointer(location, tupleSize, type, GL_TRUE, stride,
    reinterpret_cast<const void *>(intptr_t(offset)));
  }

void shader_program::set_attribute_array(const char* name, const GLfloat* values, int tupleSize, int stride)
  {
  GLint location = glGetAttribLocation(_program_id, name);;
  glVertexAttribPointer(location, tupleSize, GL_FLOAT, GL_FALSE, stride, values);
  }

void shader_program::set_attribute_array(const char* name, GLenum type, const void* values, int tupleSize, int stride)
  {
  GLint location = glGetAttribLocation(_program_id, name);;
  glVertexAttribPointer(location, tupleSize, type, GL_TRUE, stride, values);
  }

void shader_program::set_attribute_buffer(const char* name, GLenum type, int offset, int tupleSize, int stride)
  {
  GLint location = glGetAttribLocation(_program_id, name);;
  glVertexAttribPointer(location, tupleSize, type, GL_TRUE, stride,
    reinterpret_cast<const void *>(intptr_t(offset)));
  }

void shader_program::set_attribute_value(int location, const GLfloat value)
  {
  glVertexAttrib1f(location, value);
  }

void shader_program::set_attribute_value(int location, const GLfloat x, const GLfloat y)
  {
  glVertexAttrib2f(location, x, y);
  }

void shader_program::set_attribute_value(int location, const GLfloat x, const GLfloat y, const GLfloat z)
  {
  glVertexAttrib3f(location, x, y, z);
  }

void shader_program::set_attribute_value(int location, const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w)
  {
  glVertexAttrib4f(location, x, y, z, w);
  }

void shader_program::set_attribute_value(const char* name, const GLfloat value)
  {
  GLint location = glGetAttribLocation(_program_id, name);
  glVertexAttrib1f(location, value);
  }

void shader_program::set_attribute_value(const char* name, const GLfloat x, const GLfloat y)
  {
  GLint location = glGetAttribLocation(_program_id, name);
  glVertexAttrib2f(location, x, y);
  }

void shader_program::set_attribute_value(const char* name, const GLfloat x, const GLfloat y, const GLfloat z)
  {
  GLint location = glGetAttribLocation(_program_id, name);
  glVertexAttrib3f(location, x, y, z);
  }

void shader_program::set_attribute_value(const char* name, const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w)
  {
  GLint location = glGetAttribLocation(_program_id, name);
  glVertexAttrib4f(location, x, y, z, w);
  }

void shader_program::set_uniform_value(int location, GLint value)
  {
  glUniform1i(location, value);
  }

void shader_program::set_uniform_value(int location, GLuint value)
  {
  glUniform1i(location, value);
  }

void shader_program::set_uniform_value(int location, const GLfloat value)
  {
  glUniform1f(location, value);
  }

void shader_program::set_uniform_value(int location, const GLfloat x, const GLfloat y)
  {
  glUniform2f(location, x, y);
  }

void shader_program::set_uniform_value(int location, const GLfloat x, const GLfloat y, const GLfloat z)
  {
  glUniform3f(location, x, y, z);
  }

void shader_program::set_uniform_value(int location, const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w)
  {
  glUniform4f(location, x, y, z, w);
  }

void shader_program::set_uniform_value(const char* name, GLint value)
  {
  GLint location = glGetUniformLocation(_program_id, name);
  glUniform1i(location, value);
  }

void shader_program::set_uniform_value(const char* name, GLuint value)
  {
  GLint location = glGetUniformLocation(_program_id, name);
  glUniform1i(location, value);
  }

void shader_program::set_uniform_value(const char* name, const GLfloat value)
  {
  GLint location = glGetUniformLocation(_program_id, name);
  glUniform1f(location, value);
  }

void shader_program::set_uniform_value(const char* name, const GLfloat x, const GLfloat y)
  {
  GLint location = glGetUniformLocation(_program_id, name);
  glUniform2f(location, x, y);
  }

void shader_program::set_uniform_value(const char* name, const GLfloat x, const GLfloat y, const GLfloat z)
  {
  GLint location = glGetUniformLocation(_program_id, name);
  glUniform3f(location, x, y, z);
  }

void shader_program::set_uniform_value(const char* name, const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w)
  {
  GLint location = glGetUniformLocation(_program_id, name);
  glUniform4f(location, x, y, z, w);
  }

void shader_program::set_uniform_value_array(int location, const GLint* values, int count)
  {
  glUniform1iv(location, count, values);
  }

void shader_program::set_uniform_value_array(int location, const GLuint* values, int count)
  {
  const GLint *iv = reinterpret_cast<const GLint *>(values);
  glUniform1iv(location, count, iv);
  }

void shader_program::set_uniform_value_array(int location, const GLfloat* values, int count, int tupleSize)
  {
  if (tupleSize == 1)
    glUniform1fv(location, count, values);
  else if (tupleSize == 2)
    glUniform2fv(location, count, values);
  else if (tupleSize == 3)
    glUniform3fv(location, count, values);
  else if (tupleSize == 4)
    glUniform4fv(location, count, values);
  }

void shader_program::set_uniform_value_array(const char* name, const GLint* values, int count)
  {
  GLint location = glGetUniformLocation(_program_id, name);
  glUniform1iv(location, count, values);
  }

void shader_program::set_uniform_value_array(const char* name, const GLuint* values, int count)
  {
  GLint location = glGetUniformLocation(_program_id, name);
  const GLint *iv = reinterpret_cast<const GLint *>(values);
  glUniform1iv(location, count, iv);
  }

void shader_program::set_uniform_value_array(const char* name, const GLfloat* values, int count, int tupleSize)
  {
  GLint location = glGetUniformLocation(_program_id, name);

  if (tupleSize == 1)
    glUniform1fv(location, count, values);
  else if (tupleSize == 2)
    glUniform2fv(location, count, values);
  else if (tupleSize == 3)
    glUniform3fv(location, count, values);
  else if (tupleSize == 4)
    glUniform4fv(location, count, values);
  }

void shader_program::set_uniform_matrix4x4(const char *name, const float *values, int count)
  {
  GLint location = glGetUniformLocation(_program_id, name);
  const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
  glUniformMatrix4fv(location, count, GL_FALSE, fv);
  }

GLint shader_program::uniform_location(const char* name)
  {
  return glGetUniformLocation(_program_id, name);
  }

GLint shader_program::uniform_location(const std::string& name)
  {
  return glGetUniformLocation(_program_id, name.c_str());
  }
