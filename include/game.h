#pragma once

#include <memory>

class Window;
class Input;
class Renderer;

class Game {
public:
	Game();
	virtual ~Game();

	void run();

private:
	std::unique_ptr<Window> window;
	std::unique_ptr<Input> input;
        std::unique_ptr<Renderer> renderer;
};
