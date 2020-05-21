#include "SteeringManager.h"

#include "Components/Components.h"

#include <iostream>

#include "helper.h"

SteeringManager::SteeringManager(entt::registry* registry, entt::entity owner): m_registry(registry),
                                                                                m_owner(owner),
                                                                                m_wanderLastOrientation(0.0f, 0.0f) { }

glm::vec2 SteeringManager::calculate() {
    //if seek_on / if arrive_on / if evade_on ...
    glm::vec2 totalForce = glm::vec2(0.0f, 0.0f);
    totalForce += wander();

    return totalForce;
}

//Strategy pattern? i.e for each behavior its own class
glm::vec2 SteeringManager::seek(glm::vec2 target) {
    auto& transformComponent = m_registry->get<Transform>(m_owner);
    auto& physicsComponent = m_registry->get<Physics>(m_owner);

    glm::vec2 desiredVelocity = safeNormalize(target - transformComponent.position) * physicsComponent.maxSpeed;
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


glm::vec2 SteeringManager::arrive(glm::vec2 target,
                                  Deceleration deceleration) {
    auto& transformComponent = m_registry->get<Transform>(m_owner);
    auto& physicsComponent = m_registry->get<Physics>(m_owner);

    glm::vec2 direction = target - transformComponent.position;
    float distance = glm::distance(transformComponent.position, target);
    float dec = 0.3f * int(deceleration);

    float speed = distance / dec;

    glm::vec2 desiredVelocity = (direction / distance) * speed;
    return desiredVelocity - physicsComponent.velocity;
}

glm::vec2 SteeringManager::pursuit(entt::entity target) {
    auto& transformComponent = m_registry->get<Transform>(m_owner);
    auto& targetTransformComponent = m_registry->get<Transform>(target);

    auto& physicsComponent = m_registry->get<Physics>(m_owner);
    auto& targetPhysicsComponent = m_registry->get<Physics>(target);

    glm::vec2 direction = targetTransformComponent.position - transformComponent.position;

    //Angle between target's and owner's heading vectors
    float relativeHeading = glm::dot(orientationToVec(transformComponent.angle),
                                     orientationToVec(targetTransformComponent.angle));

    //Angle between owner's heading and direction vectors
    float relativeMovingDirection = glm::dot(orientationToVec(transformComponent.angle),
                                             safeNormalize(direction));

    //If angle between target and owner heading vectors lower than 18 degs
    //and owner heading towards direction
    if(relativeHeading > 0.95f && relativeMovingDirection > 0.0f) {
        return seek(targetTransformComponent.position);
    }


    float distance = glm::length(direction);
    float totalSpeed = glm::length(physicsComponent.velocity) + glm::length(targetPhysicsComponent.velocity);
    float predictTime = 0.0f;

    if(totalSpeed > MINIMAL_SPEED) {
        predictTime = distance / totalSpeed;
    }

    return seek(targetTransformComponent.position + targetPhysicsComponent.velocity * predictTime);
}

glm::vec2 SteeringManager::evade(entt::entity target) {
    auto& transformComponent = m_registry->get<Transform>(m_owner);
    auto& targetTransformComponent = m_registry->get<Transform>(target);

    auto& physicsComponent = m_registry->get<Physics>(m_owner);
    auto& targetPhysicsComponent = m_registry->get<Physics>(target);

    float distance = glm::distance(transformComponent.position, targetTransformComponent.position);
    float totalSpeed = glm::length(targetPhysicsComponent.velocity) + glm::length(physicsComponent.velocity);

    float predictTime = 0.0f;
    if(totalSpeed > MINIMAL_SPEED)
        predictTime = distance / totalSpeed;

    return flee(targetTransformComponent.position + targetPhysicsComponent.velocity * predictTime);
}

glm::vec2 SteeringManager::wander() {
    //TODO: Get/Set wander parameters
    static float wanderOffset = 50.0f;
    static float wanderRadius = 10.0f;
    static float wanderMaxLength = 1.0f;


    //For better experience
    //wanderOffset = (std::sin(SDL_GetTicks() / 1000.0f) + 1.0f) * 0.5f * 40.0f + 10.0f;
    //wanderMaxLength = (std::sin(SDL_GetTicks() / 5000.0f) + 1.0f) * 5.0f;

    auto& transformComponent = m_registry->get<Transform>(m_owner);
    auto& physicsComponent = m_registry->get<Physics>(m_owner);

    glm::vec2 randDirection = glm::vec2(drand48() - drand48(), drand48() - drand48());
    randDirection = safeNormalize(randDirection) * wanderMaxLength;

    m_wanderLastOrientation = safeNormalize(m_wanderLastOrientation + randDirection) * wanderRadius;
    //std::cout << m_wanderLastOrientation.x << " " << m_wanderLastOrientation.y << std::endl;

    glm::vec2 targetPosition = transformComponent.position +
                               safeNormalize(physicsComponent.velocity) * wanderOffset +
                               m_wanderLastOrientation;
    return seek(targetPosition);
}

glm::vec2 SteeringManager::obstacleAvoiding() {

    Transform& transform = m_registry->get<Transform>(m_owner);
    Physics& physics = m_registry->get<Physics>(m_owner);

    glm::vec2 ownerHeading = orientationToVec(transform.angle);

    entt::entity closestObstacle = entt::null;

    auto obstaclesView = m_registry->view<Obstacle, Transform, Physics>();
    for(auto obstacle: obstaclesView) {
        Transform& obstacleTransform = m_registry->get<Transform>(obstacle);
        Physics& obstaclePhysics = m_registry->get<Physics>(obstacle);

        glm::vec2 obstacleOwnerSpace = convertToLocal(ownerHeading, transform.position, obstacleTransform.position);

        if(obstacleOwnerSpace.x > 0) {  // Else obstacle is behind us and we don't care

        }
    }
}

float vecToOrientation(glm::vec2 vector) {
    return glm::degrees(std::atan2(vector.y, vector.x));
}

glm::vec2 safeNormalize(glm::vec2 vector) {
    if(glm::length(vector) > MINIMAL_SPEED)
        return glm::normalize(vector);

    return glm::vec2(0.0f, 0.0f);
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

