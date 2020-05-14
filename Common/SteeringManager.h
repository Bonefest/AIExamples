#ifndef STEERINGMANAGER_H_INCLUDED
#define STEERINGMANAGER_H_INCLUDED

#include <glm/glm.hpp>
#include "entt.hpp"

float vecToOrientation(glm::vec2 vector);
glm::vec2 orientationToVec(float angle);
glm::vec2 wrapVector(glm::vec2 vector, float maxLength);

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

private:
    entt::registry* m_registry;
    entt::entity m_owner;
};


#endif // STEERINGMANAGER_H_INCLUDED
