#ifndef STEERINGMANAGER_H_INCLUDED
#define STEERINGMANAGER_H_INCLUDED

#include <glm/glm.hpp>
#include "entt.hpp"

float vecToOrientation(glm::vec2 vector);

glm::vec2 safeNormalize(glm::vec2 vector);
glm::vec2 orientationToVec(float angle);
glm::vec2 wrapVector(glm::vec2 vector, float maxLength);

const float MINIMAL_SPEED = 0.01f;

class SteeringManager {
public:
    enum Deceleration { FAST = 1, MEDIUM, SLOW };

    entt::entity target;

    SteeringManager(entt::registry* registry, entt::entity owner);

    glm::vec2 calculate();

    //Strategy pattern? i.e for each behavior its own class
    glm::vec2 seek(glm::vec2 target);
    glm::vec2 flee(glm::vec2 target);
    glm::vec2 arrive(glm::vec2 target, Deceleration deceleration);

    glm::vec2 pursuit(entt::entity target);
    glm::vec2 evade(entt::entity target);

    glm::vec2 wander();

    glm::vec2 obstacleAvoiding();
    glm::vec2 obstacleAvoiding2();

    glm::vec2 wallAvoidance();

    glm::vec2 interpose(entt::entity targetA, entt::entity targetB);
    glm::vec2 hide(entt::entity target);

private:
    glm::vec2 findHidingSpot(glm::vec2 obstaclePosition,
                             float obstacleRadius,
                             glm::vec2 targetPosition);

    entt::registry* m_registry;
    entt::entity m_owner;

    glm::vec2 m_wanderLastOrientation;
};


#endif // STEERINGMANAGER_H_INCLUDED
