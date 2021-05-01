#pragma once

#include <GL/glew.h>

class depthtexture
  {
  public:
    depthtexture();

    ~depthtexture();

    void create(GLint w, GLint h);

    void bind_to_channel(GLint channel);
    void release();

    GLint width() const { return _width; }
    GLint height() const { return _height; }

    GLuint texture_id() const { return _texture_id; }

  private:
    GLuint _texture_id;
    GLint _width, _height;
  };
