#pragma once

#include <GL/glew.h>

class render_buffer
  {
  public:
    render_buffer();
    ~render_buffer();

    void create();
    void bind();
    void release();
    void destroy();

    bool is_created() const { return _render_buffer_id != 0; }
    GLuint object_id() const { return _render_buffer_id; }

  private:
    GLuint _render_buffer_id;
  };
