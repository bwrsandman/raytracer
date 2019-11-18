#include "input.h"
#include <SDL.h>

Input::Input():quit(false) {}
Input::~Input() = default;


void Input::run(){
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			quit = true;
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				quit = true;
				break;
			}
			break;
		}
	}
}

void Input::update_camera(Camera& camera) const{}
void Input::update_ui(Ui& ui) const {}

bool Input::should_quit() const {
	return quit;
}