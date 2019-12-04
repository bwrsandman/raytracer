#include "game.h"

#include "input.h"
#include "renderer.h"
#include "scene.h"
#include "ui.h"
#include "window.h"

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

Game::Game() {
  window = std::make_unique<Window>("Whitted_Raytracing", 800, 600);
  input = std::make_unique<Input>();
  renderer = Renderer::create(Renderer::Type::Whitted, *window);
  ui = std::make_unique<Ui>(window->get_native_handle());
  scene = Scene::load_cornel_box();
}

Game::~Game() = default;

bool
Game::main_loop()
{
  take_timestamp();
  auto delta_time = get_delta_time();
  uint16_t width, height;
  window->get_dimensions(width, height);
  renderer->set_backbuffer_size(width, height);
  input->run(*ui, *scene);
  ui->run(scene, delta_time);
  renderer->run(*scene);
  ui->draw();
  scene->run(width, height);
  window->swap();

  return !input->should_quit();
}

#if __EMSCRIPTEN__
void
em_main_loop_callback(void* arg)
{
  auto game = reinterpret_cast<Game*>(arg);
  if (!game->main_loop()) {
    delete game;
    emscripten_cancel_main_loop();
  }
}
#endif

void
Game::run()
{
  frame_end = std::chrono::high_resolution_clock::now();
#if __EMSCRIPTEN__
  emscripten_set_main_loop_arg(em_main_loop_callback, this, 0, true);
#else
  while (main_loop()) {
  }
#endif
}

void
Game::take_timestamp()
{
  frame_begin = frame_end;
  frame_end = std::chrono::high_resolution_clock::now();
}
std::chrono::microseconds
Game::get_delta_time() const
{
  auto dt = frame_end - frame_begin;
  return std::chrono::duration_cast<std::chrono::microseconds>(frame_end -
                                                               frame_begin);
}
