#include "settings.h"

#include "pref_file.h"


settings read_settings(const char* filename)
  {
  settings s;
  s.fullscreen = false;
  s.log_window = true; 
  s.script_window = true;
  s.controls = true;
  s.iterations_per_visualization = 1;
  s.elitarism_factor = 0.1;
  s.mutation_chance = 0.01;
  pref_file f(filename, pref_file::READ);
  f["file_open_folder"] >> s.file_open_folder;
  f["log_window"] >> s.log_window;
  f["script_window"] >> s.script_window;
  f["controls"] >> s.controls;
  f["fullscreen"] >> s.fullscreen;
  f["iterations_per_visualization"] >> s.iterations_per_visualization;
  f["elitarism_factor"] >> s.elitarism_factor;
  f["mutation_chance"] >> s.mutation_chance;
  return s;
  }

void write_settings(const settings& s, const char* filename)
  {
  pref_file f(filename, pref_file::WRITE);
  f << "file_open_folder" << s.file_open_folder;
  f << "script_window" << s.script_window;
  f << "controls" << s.controls;
  f << "fullscreen" << s.fullscreen;
  f << "iterations_per_visualization" << s.iterations_per_visualization;
  f << "elitarism_factor" << s.elitarism_factor;
  f << "mutation_chance" << s.mutation_chance;
  f.release();
  }
