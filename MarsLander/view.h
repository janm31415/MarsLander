#pragma once
#include <glew/GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <string>
#include <chrono>
#include <array>

#include "settings.h"
#include "model.h"
#include "mouse_data.h"

class buffer_object;
class shader_program;
class frame_buffer_object;
class render_buffer;
class texture;
class vertex_array_object;

class view
  {
  public:
    view();
    ~view(); 

    void loop();

  private:
    void _poll_for_events();
    void _imgui_ui();
    void _setup_blit_gl_objects(bool fullscreen);
    void _setup_gl_objects();
    void _log_window();
    void _control_window();
    void _script_window();
    void _destroy_gl_objects();
    void _destroy_blit_gl_objects();
    void _prepare_render();
    void _print_best_run_results();

  private:
    SDL_Window* _window;    
    uint32_t _w, _h, _viewport_w, _viewport_h, _viewport_pos_x, _viewport_pos_y;
    bool _quit;
    settings _settings;    
    frame_buffer_object* _fbo;  
    buffer_object* _vbo_array_blit;
    buffer_object* _vbo_index_blit;
    vertex_array_object* _vao_blit;
    shader_program* _program;   
    shader_program* _program_blit;
    mouse_data _md;
    model _m;
    std::string _script;
  };
