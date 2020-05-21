#ifndef HELPER_H_INCLUDED
#define HELPER_H_INCLUDED


#include "SDL2/SDL.h"
#include <glm/glm.hpp>
#include "entt.hpp"

glm::vec2 calculatePerp(glm::vec2 vec);
void drawCircle(SDL_Renderer* renderer, glm::vec2 position, float radius, SDL_Color color, int segments = 12);

#endif // HELPER_H_INCLUDED
