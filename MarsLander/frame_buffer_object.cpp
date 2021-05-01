#include "frame_buffer_object.h"

#include "depthtexture.h"
#include "texture.h"
#include "render_buffer.h"

#include <sstream>

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

frame_buffer_object::frame_buffer_object(bool with_depth_texture)
  : _w(0), _h(0), _texture(nullptr), _render_buffer(nullptr), _depthtexture(nullptr),
  _depth_owner(false), _with_depth_texture(with_depth_texture)
  {
  }

frame_buffer_object::~frame_buffer_object()
  {
  release();
  delete _texture;
  if (_depth_owner)
    {
    delete _render_buffer;
    delete _depthtexture;
    }
  }

void frame_buffer_object::create(GLint w, GLint h)
  {
  _depth_owner = true;

  if (!_with_depth_texture)
    {
    _render_buffer = new render_buffer();
    _render_buffer->create();
    gl_check_error("_render_buffer->create()");
    create(w, h, _render_buffer);
    }
  else
    {
    _depthtexture = new depthtexture();
    _depthtexture->create(w, h);
    gl_check_error("_depthtexture->create()");
    create(w, h, _depthtexture);
    }
  }

void frame_buffer_object::create(GLint w, GLint h, depthtexture *dt)
  {
  _with_depth_texture = true;
  _create(w, h, nullptr, dt);
  }

void frame_buffer_object::create(GLint w, GLint h, render_buffer *rb)
  {
  _with_depth_texture = false;
  _create(w, h, rb, nullptr);
  }

void frame_buffer_object::_create(GLint w, GLint h, render_buffer *rb, depthtexture *dt)
  {
  _w = w;
  _h = h;
  _texture = new texture(GL_REPEAT, GL_LINEAR);
  _texture->create(w, h);
  gl_check_error("_texture->create()");

  _texture->bind_to_channel(1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  if (_with_depth_texture)
    {
    _depthtexture = dt;
    _depthtexture->bind_to_channel(2);
    }
  else
    {
    _render_buffer = rb;
    _render_buffer->bind();
    }
  glGenFramebuffers(1, &_frame_buffer_id);
  glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer_id);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
    _texture->texture_id(), 0);
  if (_with_depth_texture)
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
      _depthtexture->texture_id(), 0);
  else
    {
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _texture->width(),
      _texture->height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
      _render_buffer->object_id());
    }

  GLenum status;
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  switch (status)
    {
    case GL_FRAMEBUFFER_COMPLETE:
    {
    break;
    }
    default:
    {
    throw std::runtime_error("frame buffer object is not complete");
    }
    }
  }


void frame_buffer_object::bind()
  {
  _texture->bind_to_channel(1);
  if (_with_depth_texture)
    _depthtexture->bind_to_channel(2);
  else
    _render_buffer->bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer_id);
  }

void frame_buffer_object::release()
  {
  if (_with_depth_texture)
    _depthtexture->release();
  else
    _render_buffer->release();
  _texture->release();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
