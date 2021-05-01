#pragma once

#include <GL/glew.h>

class texture;

class render_buffer;

class depthtexture;

class frame_buffer_object
  {
  public:
    frame_buffer_object(bool with_depth_texture = false);

    ~frame_buffer_object();

    void create(GLint w, GLint h, render_buffer *rb);

    void create(GLint w, GLint h, depthtexture *dt);

    void create(GLint w, GLint h);

    void bind();

    void release();

    texture *get_texture()
      {
      return _texture;
      }

    render_buffer *get_render_buffer()
      {
      return _render_buffer;
      }

    depthtexture *get_depth_texture()
      {
      return _depthtexture;
      }

    GLuint frame_buffer_id() const
      {
      return _frame_buffer_id;
      }

    GLint width() const
      {
      return _w;
      }

    GLint height() const
      {
      return _h;
      }

  private:
    void _create(GLint w, GLint h, render_buffer *rb, depthtexture *dt);

  private:
    int _w, _h;
    texture *_texture;
    render_buffer *_render_buffer;
    depthtexture *_depthtexture;
    GLuint _frame_buffer_id;
    bool _depth_owner;
    bool _with_depth_texture;
  };

