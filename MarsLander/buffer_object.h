#pragma once

#include <GL/glew.h>

class buffer_object
  {
  public:
    buffer_object();
    buffer_object(GLenum type);
    ~buffer_object();

    void allocate(const void *data, int count);

    void create();
    void bind();

    bool is_created() const { return _buffer_object_id != 0; }

    void release();
    void destroy();

    void set_usage_pattern(GLenum pattern) { _pattern = pattern; }

    GLuint buffer_id() const { return _buffer_object_id; }
    GLenum type() const { return _type; }
    GLenum usage_pattern() const { return _pattern; }

  private:
    GLenum _type;
    GLenum _pattern;
    GLuint _buffer_object_id;
  };
