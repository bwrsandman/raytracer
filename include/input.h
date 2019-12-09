#pragma once

#include <chrono>

class Scene;
class Ui;

class Input {
public:
  Input();
  virtual ~Input();
  void run(Ui& ui, Scene& scene, std::chrono::microseconds& dt);
  bool should_quit() const;

private:
  bool quit;
};
