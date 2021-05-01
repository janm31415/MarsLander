#pragma once

#include "shader.h"

class shader_program
  {
  public:
    shader_program();
    ~shader_program();

    const shader *vertex_shader() const { return _vertex_shader; }
    const shader *fragment_shader() const { return _fragment_shader; }

    bool add_shader(shader *shader);
    bool add_shader_from_source(shader::shader_type type, const char* source);
    bool add_shader_from_source(shader::shader_type type, const std::string& source);

    void remove_all_shaders();

    bool create();
    bool link();
    bool bind();
    void release();

    bool is_linked() const { return _linked; }
    const std::string& log() const { return _log; }
    GLuint program_id() const { return _program_id; }

    GLint attribute_location(const char* name);
    GLint attribute_location(const std::string& name);

    void bind_attribute_location(const char* name, int location);
    void bind_attribute_location(const std::string& name, int location);

    void disable_attribute_array(int location);
    void disable_attribute_array(const char* name);
    void disable_attribute_array(const std::string& name);

    void enable_attribute_array(int location);
    void enable_attribute_array(const char* name);
    void enable_attribute_array(const std::string& name);

    void set_attribute_array(int location, const GLfloat* values, int tupleSize, int stride);
    void set_attribute_array(int location, GLenum type, const void* values, int tupleSize, int stride);

    void set_attribute_buffer(int location, GLenum type, int offset, int tupleSize, int stride);

    void set_attribute_array(const char* name, const GLfloat* values, int tupleSize, int stride);
    void set_attribute_array(const char* name, GLenum type, const void* values, int tupleSize, int stride);

    void set_attribute_buffer(const char* name, GLenum type, int offset, int tupleSize, int stride);

    void set_attribute_value(int location, GLfloat value);
    void set_attribute_value(int location, GLfloat x, GLfloat y);
    void set_attribute_value(int location, GLfloat x, GLfloat y, GLfloat z);
    void set_attribute_value(int location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

    void set_attribute_value(const char* name, GLfloat value);
    void set_attribute_value(const char* name, GLfloat x, GLfloat y);
    void set_attribute_value(const char* name, GLfloat x, GLfloat y, GLfloat z);
    void set_attribute_value(const char* name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

    void set_uniform_value(int location, GLint value);
    void set_uniform_value(int location, GLuint value);
    void set_uniform_value(int location, GLfloat value);
    void set_uniform_value(int location, GLfloat x, GLfloat y);
    void set_uniform_value(int location, GLfloat x, GLfloat y, GLfloat z);
    void set_uniform_value(int location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

    void set_uniform_value(const char* name, GLint value);
    void set_uniform_value(const char* name, GLuint value);
    void set_uniform_value(const char* name, GLfloat value);
    void set_uniform_value(const char* name, GLfloat x, GLfloat y);
    void set_uniform_value(const char* name, GLfloat x, GLfloat y, GLfloat z);
    void set_uniform_value(const char* name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

    void set_uniform_value_array(int location, const GLint* values, int count);
    void set_uniform_value_array(int location, const GLuint* values, int count);
    void set_uniform_value_array(int location, const GLfloat* values, int count, int tupleSize);

    void set_uniform_value_array(const char* name, const GLint* values, int count);
    void set_uniform_value_array(const char* name, const GLuint* values, int count);
    void set_uniform_value_array(const char* name, const GLfloat* values, int count, int tupleSize);

    void set_uniform_matrix4x4(const char *name, const float *values, int count);

    GLint uniform_location(const char* name);
    GLint uniform_location(const std::string& name);

  private:
    int _linked;
    GLuint _program_id;

    std::string _log;
    shader* _vertex_shader;
    shader* _fragment_shader;
  };
