#pragma once

class Scene;
class Ui;

class Input {
public:
  Input();
  virtual ~Input();
  void run(Ui& ui, Scene& scene);
  bool should_quit() const;

private:
  bool quit;
};
