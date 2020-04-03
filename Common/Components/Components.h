#ifndef COMPONENTS_H_INCLUDED
#define COMPONENTS_H_INCLUDED

#include <memory>
using std::shared_ptr;

#include "glm/glm.hpp"

struct Transform {
    explicit Transform(glm::vec2 objPosition, glm::vec2 objSize, float objScale, float objAngle): position(objPosition),
                                                                                                  size(objSize),
                                                                                                  scale(objScale),
                                                                                                  angle(objAngle) { }

    glm::vec2 position;
    glm::vec2 size;
    float scale;
    float angle;
};

struct Renderable {
    explicit Renderable(SDL_Texture* text): texture(text) { }
    SDL_Texture* texture;
};

struct Kinematic {
    Kinematic(float objMaxSpeed): maxSpeed(objMaxSpeed) { }
    glm::vec2 velocity;
    float maxSpeed;

    float angularSpeed;
};

#include "../BehaviourManager.h"

struct AI {
    explicit AI(shared_ptr<BehaviourManager> bmanager) : manager(bmanager) { }
    shared_ptr<BehaviourManager> manager;
};

#endif // COMPONENTS_H_INCLUDED