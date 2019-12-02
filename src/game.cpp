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
  uint16_t width, height;
  window->get_dimensions(width, height);
  renderer->set_backbuffer_size(width, height);
  scene->run(width, height);
  input->run(*ui, *scene);
  ui->run(*scene);
  renderer->run(*scene);
  ui->draw();
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
#if __EMSCRIPTEN__
  emscripten_set_main_loop_arg(em_main_loop_callback, this, 0, true);
#else
  while (main_loop()) {
  }
#endif
}
