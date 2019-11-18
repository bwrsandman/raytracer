#pragma once

#include <memory>

class Window;
class Input;

class Game {
public:
	Game();
	virtual ~Game();

	void run();

private:
	std::unique_ptr<Window> window;
	std::unique_ptr<Input> input;
};