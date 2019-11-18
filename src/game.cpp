#include "game.h"
#include "window.h"
#include "input.h"

Game::Game() {
	window = std::make_unique<Window>("Whitted_Raytracing", 800, 600);
	input = std::make_unique <Input>();
}
Game::~Game() = default;

void Game::run() {
	while (!input->should_quit()) {
		input->run();
	}
}
