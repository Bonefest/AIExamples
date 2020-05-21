#ifndef HELPER_H_INCLUDED
#define HELPER_H_INCLUDED


#include "SDL2/SDL.h"
#include <glm/glm.hpp>
#include "entt.hpp"

glm::vec2 calculatePerp(glm::vec2 vec);
void drawCircle(SDL_Renderer* renderer, glm::vec2 position, float radius, SDL_Color color, int segments = 12);

glm::vec2 convertToLocal(glm::vec2 ownerHeadingVec, glm::vec2 ownerPos, glm::vec3 targetPos);
glm::vec2 convertToWorld(glm::vec2 ownerHeadingVec, glm::vec2 ownerPos, glm::vec3 targetPos);

#endif // HELPER_H_INCLUDED
