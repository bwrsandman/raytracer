#pragma once

#include <cstdint>
#include <string>

struct SDL_Window;

class Window {
public:
	Window(std::string name, uint16_t width, uint16_t height);
	virtual ~Window();


private:


	SDL_Window* handle;

};