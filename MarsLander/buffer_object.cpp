#include "buffer_object.h"

buffer_object::buffer_object()
  : buffer_object(GL_VERTEX_ARRAY)
  {
  }

buffer_object::buffer_object(GLenum type)
  : _type(type),
  _pattern(GL_STATIC_DRAW),
  _buffer_object_id(0)
  {
  }

buffer_object::~buffer_object()
  {
  destroy();
  }

void buffer_object::allocate(const void *data, int count)
  {
  if (!is_created())
    return;

  glBufferData(_type, count, data, _pattern);
  }

void buffer_object::create()
  {
  glGenBuffers(1, &_buffer_object_id);
  }

void buffer_object::bind()
  {
  if (_buffer_object_id != 0)
    glBindBuffer(_type, _buffer_object_id);
  }

void buffer_object::release()
  {
  glBindBuffer(_type, 0);
  }

void buffer_object::destroy()
  {
  if (_buffer_object_id != 0) 
    {
    glDeleteBuffers(1, &_buffer_object_id);
    _buffer_object_id = 0;
    }
  }

