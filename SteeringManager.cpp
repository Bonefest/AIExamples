#include "SteeringManager.h"

#include "Components/Components.h"

SteeringManager::SteeringManager(entt::registry* registry, entt::entity owner): m_registry(registry),
                                                                   m_owner(owner) { }

glm::vec2 SteeringManager::calculate() {
    //if seek_on / if arrive_on / if evade_on ...
    glm::vec2 totalForce = glm::vec2(0.0f, 0.0f);
    totalForce += seek(glm::vec2(200.0f, 200.0f));

    return totalForce;
}

//Strategy pattern? i.e for each behavior its own class
glm::vec2 SteeringManager::seek(glm::vec2 target) {
    auto& transformComponent = m_registry->get<Transform>(m_owner);
    auto& physicsComponent = m_registry->get<Physics>(m_owner);

    glm::vec2 desiredVelocity = glm::normalize(target - transformComponent.position) * physicsComponent.maxSpeed;
    return desiredVelocity - physicsComponent.velocity;
}



float vecToOrientation(glm::vec2 vector) {
    return glm::degrees(std::atan2(vector.y, vector.x));
}

glm::vec2 orientationToVec(float angle) {
    return glm::vec2(std::cos(angle), std::sin(angle));
}

glm::vec2 wrapVelocity(glm::vec2 velocity, float maxSpeed) {
    if(glm::length(velocity) > maxSpeed) {
        velocity = glm::normalize(velocity) * maxSpeed;
    }

    return velocity;
}

