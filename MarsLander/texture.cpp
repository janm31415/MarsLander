#include "texture.h"

texture::texture(GLenum wrapMode, GLenum filterMode)
  : _wrap_mode(wrapMode),
  _filter_mode(filterMode),
  _texture_id(0), _width(0), _height(0)
  {
  glGenTextures(1, &_texture_id);
  }

texture::~texture()
  {
  glDeleteTextures(1, &_texture_id);
  }

void texture::fill_pixels(GLubyte *pixels)
  {
  glBindTexture(GL_TEXTURE_2D, _texture_id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  }

void texture::create(int w, int h)
  {
  glBindTexture(GL_TEXTURE_2D, _texture_id);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);

  _width = w;
  _height = h;
  }

void texture::bind_to_channel(int channel)
  {
  glActiveTexture(GL_TEXTURE0 + channel);
  glBindTexture(GL_TEXTURE_2D, _texture_id);

  if (_wrap_mode == GL_REPEAT) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
  else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

  if (_filter_mode == GL_NEAREST) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
  else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
  }

void texture::release()
  {
  glBindTexture(GL_TEXTURE_2D, 0);
  }
