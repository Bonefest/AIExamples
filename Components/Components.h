#ifndef COMPONENTS_H_INCLUDED
#define COMPONENTS_H_INCLUDED

#include "glm/glm.hpp"

struct Transform {

    glm::vec2 position;
    glm::vec2 size;
    float scale;
    float angle;
};

struct Renderable {

    SDL_Texture* texture;
};

#endif // COMPONENTS_H_INCLUDED
