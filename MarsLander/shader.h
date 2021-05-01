#pragma once

#include <string>
#include <gl/glew.h>

enum shader_type {
  Vertex = 1 << 1,
  Fragment = 1 << 2
  };

class shader
  {
  public:
    typedef enum shader_type shader_type;

    shader(shader::shader_type shader_type);
    ~shader();

    bool compile_source_code(const char* source);
    bool compile_source_code(const std::string& source);

    bool is_compiled() const { return _compiled; }

    GLuint shader_id() const { return _shader_id; }
    shader::shader_type shadertype() const { return _shader_type; }

    std::string source_code() const { return _source_code; }
    std::string log() const { return _log; }

  protected:
    bool create();
    void destroy();
    bool compile(const char* source);

  private:
    friend class shader_program;

    bool _compiled;
    GLuint _shader_id;
    shader_type _shader_type;

    std::string _log;
    std::string _source_code;
  };
