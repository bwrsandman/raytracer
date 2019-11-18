#include "game.h"

#include "input.h"
#include "renderer_whitted.h"
#include "window.h"

Game::Game() {
  window = std::make_unique<Window>("Whitted_Raytracing", 800, 600);
  input = std::make_unique<Input>();
  renderer = std::make_unique<RendererWhitted>(*window);
}
Game::~Game() = default;

void Game::run() {
  auto now = std::chrono::high_resolution_clock::now();
  while (!input->should_quit()) {
    auto dt = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::high_resolution_clock::now() - now);
    renderer->run(dt);
    input->run();
    window->swap();
    now = std::chrono::high_resolution_clock::now();
  }
}
