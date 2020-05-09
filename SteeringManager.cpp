#include "SteeringManager.h"

#include "Components/Components.h"

SteeringManager::SteeringManager(entt::registry* registry, entt::entity owner): m_registry(registry),
                                                                   m_owner(owner) { }

glm::vec2 SteeringManager::calculate() {
    //if seek_on / if arrive_on / if evade_on ...
    glm::vec2 totalForce = glm::vec2(0.0f, 0.0f);
    totalForce += arrive(target);

    return totalForce;
}

//Strategy pattern? i.e for each behavior its own class
glm::vec2 SteeringManager::seek(glm::vec2 target) {
    auto& transformComponent = m_registry->get<Transform>(m_owner);
    auto& physicsComponent = m_registry->get<Physics>(m_owner);

    glm::vec2 desiredVelocity = glm::normalize(target - transformComponent.position) * physicsComponent.maxSpeed;
    return desiredVelocity - physicsComponent.velocity;
}

glm::vec2 SteeringManager::flee(glm::vec2 target) {
    auto& transformComponent = m_registry->get<Transform>(m_owner);
    auto& physicsComponent = m_registry->get<Physics>(m_owner);

    float distance = glm::distance(transformComponent.position, target);
    if(distance > 100.0f) {
        return glm::vec2(0.0f, 0.0f); //No force required
    }

    glm::vec2 desiredVelocity = physicsComponent.maxSpeed * (transformComponent.position - target) / distance;
    return desiredVelocity - physicsComponent.velocity;
}


glm::vec2 SteeringManager::arrive(glm::vec2 target) {
    auto& transformComponent = m_registry->get<Transform>(m_owner);
    auto& physicsComponent = m_registry->get<Physics>(m_owner);

    glm::vec2 direction = target - transformComponent.position;
    float distance = glm::distance(transformComponent.position, target);
    float speed = distance; // * constant

    glm::vec2 desiredVelocity = (direction / distance) * speed;
    return desiredVelocity - physicsComponent.velocity;
}

float vecToOrientation(glm::vec2 vector) {
    return glm::degrees(std::atan2(vector.y, vector.x));
}

glm::vec2 orientationToVec(float angle) {
    return glm::vec2(std::cos(angle), std::sin(angle));
}

glm::vec2 wrapVector(glm::vec2 vector, float maxLength) {
    if(glm::length(vector) > maxLength) {
        vector = glm::normalize(vector) * maxLength;
    }

    return vector;
}

