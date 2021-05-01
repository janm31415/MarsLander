#include "view.h"
#include <iostream>
#include <SDL_syswm.h>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "imguifilesystem.h"

#include <glew/GL/glew.h>

#include <stdexcept>
#include <chrono>
#include <string>
#include <fstream>
#include <streambuf>
#include <ctime>
#include <iomanip>
#include <cmath>

#include "buffer_object.h"
#include "frame_buffer_object.h"
#include "shader_program.h"
#include "render_buffer.h"
#include "vertex_array_object.h"
#include "texture.h"
#include "logging.h"

#define V_W 800
#define V_H 450
#define V_X 50
#define V_Y 50

namespace
  {
  void gl_check_error(const char* txt)
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

view::view() : _w(1600), _h(900), _quit(false),
_vbo_array_blit(nullptr), _vbo_index_blit(nullptr), _vao_blit(nullptr),
_program(nullptr), _program_blit(nullptr), _viewport_w(V_W), _viewport_h(V_H), _fbo(nullptr),
_viewport_pos_x(V_X), _viewport_pos_y(V_Y)
  {
  // Setup window
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  
  //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

  _window = SDL_CreateWindow("Mars Lander",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    _w, _h,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
  if (!_window)
    throw std::runtime_error("SDL can't create a window");

  SDL_GLContext gl_context = SDL_GL_CreateContext(_window);
  SDL_GL_SetSwapInterval(1); // Enable vsync


  glewExperimental = true;
  GLenum err = glewInit();
  if (GLEW_OK != err)
    throw std::runtime_error("GLEW initialization failed");
  glGetError(); // hack https://stackoverflow.com/questions/36326333/openglglfw-glgenvertexarrays-returns-gl-invalid-operation

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  // Setup Platform/Renderer bindings
  ImGui_ImplSDL2_InitForOpenGL(_window, gl_context);
  ImGui_ImplOpenGL3_Init();

  // Setup Style
  ImGui::StyleColorsDark();
  ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive];

  SDL_GL_MakeCurrent(_window, gl_context);

  _settings = read_settings("marslander.cfg");

  _setup_gl_objects();
  _setup_blit_gl_objects(_settings.fullscreen);

  _md.left_dragging = false;
  _md.right_dragging = false;
  _md.right_button_down = false;
  _md.left_button_down = false;
  _md.wheel_down = false;
  _md.wheel_mouse_pressed = false;
  _md.mouse_x = 0.f;
  _md.mouse_y = 0.f;
  _md.prev_mouse_x = 0.f;
  _md.prev_mouse_y = 0.f;
  _md.wheel_rotation = 0.f;
  
  _script=R"(7
0 100
1000 500
1500 1500
3000 1000
4000 150
5500 150
6999 800
2500 2700 0 0 550 0 0
)";

  init_model(_m, _script);
  make_random_population(_m);
  _prepare_render();
  }


view::~view()
  {
  write_settings(_settings, "marslander.cfg");

  _destroy_gl_objects();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  SDL_DestroyWindow(_window);
  }

void view::_setup_blit_gl_objects(bool fullscreen)
  {
  _viewport_w = V_W;
  _viewport_h = V_H;
  _viewport_pos_x = V_X;
  _viewport_pos_y = V_Y;

  int pos_x = _viewport_pos_x;
  int pos_y = _viewport_pos_y;
  int w = _viewport_w;
  int h = _viewport_h;

  float width = 2.f*(float)w / (float)_w;
  float height = 2.f*(float)h / (float)_h;

  float x0 = ((float)pos_x / (float)_w)*2.f - 1.f;
  float y0 = 1.f - 2.f*((float)pos_y / (float)_h);
  float x1 = x0 + width;
  float y1 = y0 - height;



  if (fullscreen)
    {
    pos_x = 0;
    pos_y = 0;
    _viewport_w = _w;
    _viewport_h = _h;
    _viewport_pos_x = 0;
    _viewport_pos_y = 0;
    x0 = -1.f;
    y0 = 1.f;
    x1 = 1.f;
    y1 = -1.f;
    w = _w;
    h = _h;
    }

  GLfloat vertices[] = {
 x0,  y0,  0.0f,
 x1,  y0,  0.0f,
 x1,  y1,  0.0f,
 x0,  y1,  0.0f
    };

  GLuint indices[] = {
      0, 1, 2,
      0, 2, 3
    };


  _fbo = new frame_buffer_object();
  _fbo->create(_viewport_w, _viewport_h);
  gl_check_error("_fbo->create()");
  _fbo->release();

  _vao_blit = new vertex_array_object();
  _vao_blit->create();
  gl_check_error(" _vao_blit->create()");
  _vao_blit->bind();
  gl_check_error(" _vao_blit->bind()");

  _vbo_array_blit = new buffer_object(GL_ARRAY_BUFFER);
  _vbo_array_blit->create();
  gl_check_error("_vbo_array_blit->create()");
  _vbo_array_blit->bind();
  gl_check_error("_vbo_array_blit->bind()");
  _vbo_array_blit->set_usage_pattern(GL_STATIC_DRAW);
  _vbo_array_blit->allocate(vertices, sizeof(vertices));
  gl_check_error("_vbo_array_blit->allocate()");

  _vbo_index_blit = new buffer_object(GL_ELEMENT_ARRAY_BUFFER);
  _vbo_index_blit->create();
  gl_check_error("_vbo_index_blit->create()");
  _vbo_index_blit->set_usage_pattern(GL_STATIC_DRAW);
  _vbo_index_blit->bind();
  gl_check_error("_vbo_index_blit->bind()");
  _vbo_index_blit->allocate(indices, sizeof(indices));
  gl_check_error("_vbo_index_blit->allocate()");

  _vao_blit->release();
  gl_check_error(" _vao_blit->release()");

  std::string vertex_shader = R"(#version 330 core
precision mediump float;
precision mediump int;
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vUV;

void main() 
  {
  gl_Position = vec4(vPosition, 1.0f);
  }
)";
  std::string fragment_shader = R"(#version 330 core
precision mediump float;
precision mediump int;
uniform vec2      iBlitResolution;
uniform vec2      iBlitOffset;
uniform sampler2D iChannel0;

out vec4 FragColor;

void main()
{
    vec2 pos = (gl_FragCoord.xy - iBlitOffset)/iBlitResolution;
    FragColor = texture(iChannel0, pos);
}
)";

  _program_blit = new shader_program();
  _program_blit->add_shader_from_source(shader::shader_type::Vertex, vertex_shader);
  _program_blit->add_shader_from_source(shader::shader_type::Fragment, fragment_shader);
  _program_blit->link();

  _program_blit->release();
  _vbo_array_blit->release();
  _vbo_index_blit->release();
  }

void view::_setup_gl_objects()
  {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  std::string vertex_shader = R"(#version 330 core
precision mediump float;
precision mediump int;
layout (location = 0) in vec2 vPosition;

void main()
  {
  gl_Position = vec4(vPosition, 0.0, 1.0);  
  }
)";

  std::string fragment = R"(#version 330 core
precision mediump float;
precision mediump int;

out vec4 FragColor;


void main()
  {
  FragColor = vec4(1, 0, 0, 1);
  }
)";


  _program = new shader_program();
  _program->add_shader_from_source(shader::shader_type::Vertex, vertex_shader);
  _program->add_shader_from_source(shader::shader_type::Fragment, fragment);
  _program->link();

  gl_check_error("_program->link()");

  _program->release();
  }

void view::_destroy_gl_objects()
  {
  _fbo->release();
  _program->release();
  _vbo_array_blit->release();
  _vbo_index_blit->release();
  _vao_blit->release();
  _program_blit->release();
  delete _fbo;
  delete _program;
  delete _vbo_array_blit;
  delete _vbo_index_blit;
  delete _vao_blit;
  delete _program_blit;
  _fbo = nullptr;
  _program = nullptr;
  _vbo_array_blit = nullptr;
  _vbo_index_blit = nullptr;
  _vao_blit = nullptr;
  _program_blit = nullptr;
  }


void view::_destroy_blit_gl_objects()
  {
  _vbo_array_blit->release();
  _vbo_index_blit->release();
  _vao_blit->release();
  _program_blit->release();
  _fbo->release();
  delete _vbo_array_blit;
  delete _vbo_index_blit;
  delete _vao_blit;
  delete _program_blit;
  delete _fbo;
  _vbo_array_blit = nullptr;
  _vbo_index_blit = nullptr;
  _vao_blit = nullptr;
  _program_blit = nullptr;
  _fbo = nullptr;
  }

void view::_poll_for_events()
  {
  SDL_Event event;
  while (SDL_PollEvent(&event))
    {
    ImGui_ImplSDL2_ProcessEvent(&event);
    switch (event.type)
      {
      case SDL_QUIT:
      {
      _quit = true;
      break;
      }
      case SDL_WINDOWEVENT:
      {
      if (event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
        _destroy_gl_objects();
        _w = event.window.data1;
        _h = event.window.data2;
        glViewport(0, 0, _w, _h);
        _setup_gl_objects();
        _setup_blit_gl_objects(_settings.fullscreen);
        }
      break;
      }
      case SDL_MOUSEMOTION:
      {
      _md.prev_mouse_x = _md.mouse_x;
      _md.prev_mouse_y = _md.mouse_y;
      _md.mouse_x = float(event.motion.x);
      _md.mouse_y = float(event.motion.y);
      if (_settings.fullscreen)
        {
        float width_ratio = (float)_viewport_w / (float)_w;
        float height_ratio = (float)_viewport_h / (float)_h;

        _md.mouse_x *= width_ratio;
        _md.mouse_y *= height_ratio;
        }
      break;
      }
      case SDL_MOUSEBUTTONDOWN:
      {
      if (event.button.button == 2)
        {
        _md.wheel_mouse_pressed = true;
        _md.wheel_down = true;
        }
      else if (event.button.button == 1)
        {
        _md.left_dragging = true;
        _md.left_button_down = true;
        }
      else if (event.button.button == 3)
        {
        _md.right_dragging = true;
        _md.right_button_down = true;
        }
      break;
      }
      case SDL_MOUSEBUTTONUP:
      {
      if (event.button.button == 2)
        _md.wheel_mouse_pressed = false;
      else if (event.button.button == 1)
        {
        _md.left_dragging = false;
        }
      else if (event.button.button == 3)
        {
        _md.right_dragging = false;
        }
      break;
      }
      case SDL_MOUSEWHEEL:
      {
      _md.wheel_rotation += event.wheel.y;
      break;
      }
      }
    }
  }

void view::_imgui_ui()
  {
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(_window);
  ImGui::NewFrame();

  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  window_flags |= ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_MenuBar;
  window_flags |= ImGuiWindowFlags_NoBackground;
  window_flags |= ImGuiWindowFlags_NoResize;
  window_flags |= ImGuiWindowFlags_NoScrollbar;

  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2((float)_w, 10), ImGuiCond_Always);
  bool open = true;
  static bool open_script = false;
  static bool save_script = false;
  if (ImGui::Begin("MarsLander", &open, window_flags))
    {
    if (!open)
      _quit = true;
    if (ImGui::BeginMenuBar())
      {
      if (ImGui::BeginMenu("File"))
        {        
        if (ImGui::MenuItem("Load"))
          {
          open_script = true;
          }
        if (ImGui::MenuItem("Save"))
          {
          save_script = true;
          }
        if (ImGui::MenuItem("Exit"))
          {
          _quit = true;
          }
        ImGui::EndMenu();
        }

      if (ImGui::BeginMenu("Window"))
        {
        if (ImGui::MenuItem("Fullscreen", NULL, &_settings.fullscreen))
          {          
          _destroy_blit_gl_objects();
          _setup_blit_gl_objects(_settings.fullscreen);
          }
        ImGui::MenuItem("Controls", NULL, &_settings.controls);
        ImGui::MenuItem("Log window", NULL, &_settings.log_window);
        ImGui::MenuItem("Script window", NULL, &_settings.script_window);
        ImGui::EndMenu();
        }
      ImGui::EndMenuBar();
      }
    ImGui::End();
    }

  static ImGuiFs::Dialog open_script_dlg(false, true, true);
  const char* openScriptChosenPath = open_script_dlg.chooseFileDialog(open_script, _settings.file_open_folder.c_str(), ".txt", "Open script", ImVec2(-1, -1), ImVec2(50, 50));
  open_script = false;
  if (strlen(openScriptChosenPath) > 0)
    {
    _settings.file_open_folder = open_script_dlg.getLastDirectory();
    std::ifstream t(openScriptChosenPath);
    std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    _script = str;
    _prepare_render();
    }

  static ImGuiFs::Dialog save_script_dlg(false, true, true);
  const char* saveScriptChosenPath = save_script_dlg.saveFileDialog(save_script, _settings.file_open_folder.c_str(), 0, ".txt", "Save script");
  save_script = false;
  if (strlen(saveScriptChosenPath) > 0)
    {
    _settings.file_open_folder = save_script_dlg.getLastDirectory();
    std::ofstream t(saveScriptChosenPath);
    t << _script;
    t.close();
    }

  if (_settings.log_window)
    _log_window();

  if (_settings.controls)
    _control_window();

  if (_settings.script_window)
    _script_window();
  ImGui::Render();
  }

void view::_script_window()
  {
  ImGui::SetNextWindowSize(ImVec2((float)(_w - V_W - 3 * V_X), (float)(_h - 3 * V_Y - V_H)), ImGuiCond_Always);
  ImGui::SetNextWindowPos(ImVec2((float)(V_X * 2 + V_W), (float)(2 * V_Y + V_H)), ImGuiCond_Always);

  if (!ImGui::Begin("Script window", &_settings.script_window))
    {
    ImGui::End();
    return;
    }
  ImGuiInputTextFlags flags = 0;
  ImGui::InputTextMultiline("Scripting", &_script, ImVec2(-1.0f, _h - 2 * _viewport_pos_y - ImGui::GetTextLineHeight() * 6), flags);
  ImGui::End();
  }

void view::_prepare_render()
  {
  _m.delete_render_objects();
  fill_terrain_data(_m);
  simulate_population(_m);
  }

void view::_control_window()
  {
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  window_flags |= ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoBackground;
  window_flags |= ImGuiWindowFlags_NoResize;
  window_flags |= ImGuiWindowFlags_NoScrollbar;

  ImGui::SetNextWindowSize(ImVec2((float)(_w - V_W - 3 * V_X), (float)(_h/2)), ImGuiCond_Always);
  ImGui::SetNextWindowPos(ImVec2((float)(V_X * 2 + V_W), (float)(V_Y)), ImGuiCond_Always);

  if (!ImGui::Begin("Control window", &_settings.controls, window_flags))
    {
    ImGui::End();
    return;
    }

  if (ImGui::Button("Start")) {
    init_model(_m, _script);
    make_random_population(_m);
    _prepare_render();
  }

  ImGui::End();
  }

void view::_log_window()
  {
  static AppLog log;

  auto log_messages = Logging::GetInstance().pop_messages();

  if (!log_messages.empty())
    log.AddLog(log_messages.c_str());

  ImGui::SetNextWindowSize(ImVec2((float)V_W, (float)(_h - 3 * V_Y - V_H)), ImGuiCond_Always);
  ImGui::SetNextWindowPos(ImVec2((float)V_X, (float)(2 * V_Y + V_H)), ImGuiCond_Always);

  log.Draw("Log window", &_settings.log_window);
  }


void view::loop()
  {
  while (!_quit)
    {
    _poll_for_events();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _fbo->bind();
    gl_check_error("_fbo->bind()");
    glViewport(0, 0, _viewport_w, _viewport_h);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     

    _m._vao->bind();
    gl_check_error("_vao->bind()");
    _m._vbo_array->bind();
    gl_check_error("_vbo_array->bind()");

    _program->bind();
    gl_check_error("_program->bind()");

    _program->enable_attribute_array(0);
    gl_check_error("_program->enable_attribute_array(0)");
    _program->set_attribute_buffer(0, GL_FLOAT, 0, 2, sizeof(GLfloat) * 2); // x y
    gl_check_error("_program->set_attribute_buffer(0, GL_FLOAT, 0, 2, sizeof(GLfloat) * 2)");

    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)(_m.number_of_terrain_points));
    gl_check_error("glDrawArrays");
    
    _program->release();
    gl_check_error("_program->release()");
    _m._vbo_array->release();
    gl_check_error("_m._vbo_array->release()");
    _m._vao->release();
    gl_check_error("_m._vao->release()");
    
    for (int i = 0; i < _m._path_vao.size(); ++i) {
      _m._path_vao[i]->bind();
      gl_check_error("_path_vao[i]->bind()");
      _m._path_vbo_array[i]->bind();
      gl_check_error("_path_vbo_array[i]->bind()");

      _program->bind();
      gl_check_error("_program->bind()");

      _program->enable_attribute_array(0);
      gl_check_error("_program->enable_attribute_array(0)");
      _program->set_attribute_buffer(0, GL_FLOAT, 0, 2, sizeof(GLfloat) * 2); // x y
      gl_check_error("_program->set_attribute_buffer(0, GL_FLOAT, 0, 2, sizeof(GLfloat) * 2)");

      glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)(_m.current_population.front().size()));
      gl_check_error("glDrawArrays");
      
      _program->release();
      gl_check_error("_program->release()");
      _m._path_vbo_array[i]->release();
      gl_check_error("_m._path_vbo_array[i]->release()");
      _m._path_vao[i]->release();
      gl_check_error("_m._path_vao[i]->release()");
    }
    
    _fbo->release();
    gl_check_error("_fbo->release()");

    glViewport(0, 0, _w, _h);

    _vao_blit->bind();
    gl_check_error("_vao_blit->bind()");
    _fbo->get_texture()->bind_to_channel(0);
    gl_check_error("_fbo->get_texture()->bind_to_channel(0)");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _vbo_array_blit->bind();
    gl_check_error("_vbo_array_blit->bind()");
    _vbo_index_blit->bind();
    gl_check_error("_vbo_index_blit->bind()");
    _program_blit->bind();
    gl_check_error("_program_blit->bind()");
    _program_blit->enable_attribute_array(0);
    gl_check_error("_program_blit->enable_attribute_array(0)");
    _program_blit->set_attribute_buffer(0, GL_FLOAT, 0, 3, sizeof(GLfloat) * 3); // x y z
    gl_check_error(" _program_blit->set_attribute_buffer(0, GL_FLOAT, 0, 3, sizeof(GLfloat) * 3)");

    _program_blit->set_uniform_value("iBlitResolution", (GLfloat)_viewport_w, (GLfloat)_viewport_h);
    _program_blit->set_uniform_value("iBlitOffset", (GLfloat)_viewport_pos_x, (GLfloat)(_h - _viewport_pos_y));
    _program_blit->set_uniform_value("iChannel0", 0);


    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    gl_check_error("glDrawElements");

    _program_blit->release();
    _vbo_array_blit->release();
    _vbo_index_blit->release();

    _vao_blit->release();
    _fbo->get_texture()->release();

    _imgui_ui();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    SDL_GL_SwapWindow(_window);

    glGetError(); //hack
    }
  }
