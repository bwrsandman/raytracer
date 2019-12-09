#include <cstdlib>

#include "game.h"

int
main(int argc, char* argv[])
{

  Raytracer::Game game;
  game.run();

  return EXIT_SUCCESS;
}
