#pragma once

#include <gl/glew.h>

class texture
  {
  public:
    texture(GLenum wrapMode, GLenum filterMode);

    ~texture();

    void fill_pixels(GLubyte *pixels);

    void create(GLint w, GLint h);

    void bind_to_channel(GLint channel);
    void release();

    GLint width() const { return _width; }
    GLint height() const { return _height; }

    GLuint texture_id() const { return _texture_id; }

  private:
    GLenum _wrap_mode;
    GLenum _filter_mode;

    GLuint _texture_id;
    GLint _width, _height;
  };
