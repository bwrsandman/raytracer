#include "game.h"

#include "input.h"
#include "renderer.h"
#include "scene.h"
#include "ui.h"
#include "window.h"

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

using namespace Raytracer;
using namespace Raytracer::Graphics;

Game::Game() {
  constexpr uint16_t width = 800;
  constexpr uint16_t height = 600;
  window = std::make_unique<Window>("Raytracer", width, height);
  input = std::make_unique<Input>();
  renderer = Renderer::create(Renderer::Type::Gpu, window->get_native_handle());
  ui = std::make_unique<Ui>(window->get_native_handle());
  scene = Scene::load_cornell_box();
}

Game::~Game() = default;

void
Game::clean_up()
{
  renderer_metrics.clear();
  scene.reset();
  ui.reset();
  renderer.reset();
  input.reset();
  window.reset();
}

bool
Game::main_loop()
{
  take_timestamp();
  auto delta_time = get_delta_time();
  uint16_t width, height;
  window->get_dimensions(width, height);
  renderer->set_backbuffer_size(width, height);
  input->run(*ui, *scene, delta_time);
  ui->run(scene, *renderer, renderer_metrics, delta_time);
  renderer->run(*scene);
  ui->draw();
  scene->run(width, height);
  window->swap();
  renderer_metrics = renderer->evaluate_metrics();

  return !input->should_quit();
}

#if __EMSCRIPTEN__
void
em_main_loop_callback(void* arg)
{
  auto game = reinterpret_cast<Game*>(arg);
  if (!game->main_loop()) {
    game->clean_up();
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
  clean_up();
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
  return std::chrono::duration_cast<std::chrono::microseconds>(dt);
}
