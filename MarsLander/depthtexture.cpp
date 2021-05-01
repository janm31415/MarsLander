#include "depthtexture.h"

depthtexture::depthtexture()
  : _texture_id(0), _width(0), _height(0)
  {
  glGenTextures(1, &_texture_id);
  }

depthtexture::~depthtexture()
  {
  glDeleteTextures(1, &_texture_id);
  }

void depthtexture::create(int w, int h)
  {
  glBindTexture(GL_TEXTURE_2D, _texture_id);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, w, h);

  _width = w;
  _height = h;
  }

void depthtexture::bind_to_channel(int channel)
  {
  glActiveTexture(GL_TEXTURE0 + channel);
  glBindTexture(GL_TEXTURE_2D, _texture_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }

void depthtexture::release()
  {
  glBindTexture(GL_TEXTURE_2D, 0);
  }
