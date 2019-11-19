#include "game.h"

#include "input.h"
#include "renderer.h"
#include "window.h"

Game::Game() {
  window = std::make_unique<Window>("Whitted_Raytracing", 800, 600);
  input = std::make_unique<Input>();
  renderer = Renderer::create(Renderer::Type::Whitted, *window);
}
Game::~Game() = default;

void Game::run() {
  auto now = std::chrono::high_resolution_clock::now();
  uint16_t width, height;

  while (!input->should_quit()) {
    auto dt = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::high_resolution_clock::now() - now);
	
    window->get_dimensions(width, height);
    renderer->set_backbuffer_size(width, height);
    renderer->run(dt);
    input->run();
    window->swap();
    now = std::chrono::high_resolution_clock::now();
  }
}
