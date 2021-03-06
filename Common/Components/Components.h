#ifndef COMPONENTS_H_INCLUDED
#define COMPONENTS_H_INCLUDED

#include <memory>
using std::shared_ptr;

#include "SDL2/SDL.h"
#include "glm/glm.hpp"
#include "../../Path.h"

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

struct Path {
    Path(shared_ptr<IPath> t_path, bool t_needRendering = true): path(t_path),
                                                                 needRendering(t_needRendering) { }
    shared_ptr<IPath> path;
    bool needRendering;
};

struct Controllable {

};

struct Kinematic {
    Kinematic(float objMaxSpeed, float objMaxAcceleration, float objMaxAngularSpeed): velocity(0.0f, 0.0f),
                                                                                      maxSpeed(objMaxSpeed),
                                                                                      maxAcceleration(objMaxAcceleration),
                                                                                      angularSpeed(0.0f),
                                                                                      maxAngularSpeed(objMaxAngularSpeed) { }
    glm::vec2 velocity;
    float maxSpeed;
    float maxAcceleration;

    float angularSpeed;
    float maxAngularSpeed;
};

struct Physics {
    Physics(float t_maxSpeed, float t_mass, float t_maxForce, float t_maxTurnRate,float t_radius): maxSpeed(t_maxSpeed),
                                                                                                   mass(t_mass),
                                                                                                   maxForce(t_maxForce),
                                                                                                   maxTurnRate(t_maxTurnRate),
                                                                                                   radius(t_radius),
                                                                                                   velocity(0.0f, 0.0f) { }
    float maxSpeed;
    float mass;
    float maxForce;
    float maxTurnRate;

    float radius;

    glm::vec2 velocity;
};

struct Obstacle {
    Obstacle(SDL_Color t_color): color(t_color) { }

    SDL_Color color;
};


#include "Common.h"
#include "helper.h"

struct Wall {

    Wall(Line2D l_line, SDL_Color l_color): line(l_line), color(l_color) {
        normal = glm::normalize(calculatePerp(l_line.end - l_line.start));
    }

    Line2D      line;
    SDL_Color   color;


    glm::vec2   normal;
};


#include "../SteeringManager.h"

struct AI {
    explicit AI(shared_ptr<SteeringManager> bmanager) : manager(bmanager) { }
    shared_ptr<SteeringManager> manager;
};

#endif // COMPONENTS_H_INCLUDED
