#include "vertex_array_object.h"

vertex_array_object::vertex_array_object()
  : _vertex_array_object_id(0)
  {
  }

vertex_array_object::~vertex_array_object()
  {
  destroy();
  }

void vertex_array_object::create()
  {
  glGenVertexArrays(1, &_vertex_array_object_id);
  }

void vertex_array_object::bind()
  {
  glBindVertexArray(_vertex_array_object_id);
  }

void vertex_array_object::release()
  {
  glBindVertexArray(0);
  }

void vertex_array_object::destroy()
  {
  if (_vertex_array_object_id != 0) 
    {
    glDeleteVertexArrays(1, &_vertex_array_object_id);
    _vertex_array_object_id = 0;
    }
  }



