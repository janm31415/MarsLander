#include <GL/glew.h>

class vertex_array_object
  {
  public:
    vertex_array_object();
    ~vertex_array_object();

    void create();
    void bind();
    void release();
    void destroy();

    bool is_created() const { return _vertex_array_object_id != 0; }
    GLuint object_id() const { return _vertex_array_object_id; }

  private:
    GLuint _vertex_array_object_id;
  };
