#pragma once

class Camera;
class Ui;

class Input {
public:
  Input();
  virtual ~Input();
  void run(Ui& ui);
  void update_camera(Camera& camera) const;
  bool should_quit() const;

private:
  bool quit;
};
