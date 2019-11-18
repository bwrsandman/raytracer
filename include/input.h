#pragma once

class Camera;
class Ui;

class Input {
public:
	Input();
	virtual ~Input();
	void run();
	void update_camera(Camera& camera) const;
	void update_ui(Ui& ui) const;
	bool should_quit() const;
private:
	bool quit;
};