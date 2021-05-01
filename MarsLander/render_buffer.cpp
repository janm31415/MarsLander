#include "render_buffer.h"

render_buffer::render_buffer()
  : _render_buffer_id(0)
  {
  }

render_buffer::~render_buffer()
  {
  destroy();
  }

void render_buffer::create()
  {
  glGenRenderbuffersEXT(1, &_render_buffer_id);
  }

void render_buffer::bind()
  {
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _render_buffer_id);
  }

void render_buffer::release()
  {
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
  }

void render_buffer::destroy()
  {
  if (_render_buffer_id != 0)
    {
    glDeleteRenderbuffersEXT(1, &_render_buffer_id);
    _render_buffer_id = 0;
    }
  }



