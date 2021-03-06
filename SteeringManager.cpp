#include "SteeringManager.h"

#include "Components/Components.h"

#include <iostream>

#include "helper.h"
#include <cmath>

SteeringManager::SteeringManager(entt::registry* registry, entt::entity owner): m_registry(registry),
                                                                                m_owner(owner),
                                                                                m_wanderLastOrientation(0.0f, 0.0f) { }

glm::vec2 SteeringManager::calculate() {
    //if seek_on / if arrive_on / if evade_on ...
    glm::vec2 totalForce = glm::vec2(0.0f, 0.0f);
    totalForce += 0.7f * obstacleAvoiding2() + 0.3f * offsetPursuit(target, glm::vec2(-48.0f, -48.0f));
//    totalForce += wallAvoidance();
//    totalForce += obstacleAvoiding2();
//    if(glm::length(totalForce) < 0.1f)
//        totalForce += wander() * 0.2f;

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
    if(distance > 300.0f) {
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


glm::vec2 SteeringManager::obstacleAvoiding2() {
    Transform& transform = m_registry->get<Transform>(m_owner);
    Physics& physics = m_registry->get<Physics>(m_owner);

    glm::vec2 head = safeNormalize(physics.velocity);

    if(head.x * head.x + head.y * head.y < 0.01f) {
        head = orientationToVec(transform.angle);
    }


    float minBoxLength = 100.0f;
    float boxLength = minBoxLength + physics.velocity.length() / physics.maxSpeed * minBoxLength;


    entt::entity closestObstacle = entt::null;
    float closestDistance = 0.0f;

    auto obstaclesView = m_registry->view<Transform, Physics, Obstacle>();
    obstaclesView.each([&](entt::entity obstacle, Transform& obTransform, Physics& obPhysics, Obstacle& obs) {
        glm::vec2 obstacleLocalPos = convertToLocal(head, transform.position, obTransform.position);
        if(obstacleLocalPos.x < 0.0f || obstacleLocalPos.x - obPhysics.radius >= boxLength) {
            return;
        }

        float r = obPhysics.radius + physics.radius;

        if(r < std::fabs(obstacleLocalPos.y)) {
            return;
        }

        float sq = std::sqrt(r*r - obstacleLocalPos.y * obstacleLocalPos.y);
        float mn = obstacleLocalPos.x - sq;
        if(mn <= 0.0) {
            mn = obstacleLocalPos.x + sq;
        }

        if(closestObstacle == entt::null || mn < closestDistance) {
            closestObstacle = obstacle;
            closestDistance = mn;
        }
    });

    std::cout << closestDistance << "\n";

    if(closestObstacle != entt::null) {
        Transform& obTransform = m_registry->get<Transform>(closestObstacle);
        Physics& obPhysics = m_registry->get<Physics>(closestObstacle);
        Obstacle& obstacle = m_registry->get<Obstacle>(closestObstacle);
        obstacle.color = SDL_Color {255, 0, 0 };
        glm::vec2 obstacleLocalPos = convertToLocal(head, transform.position, obTransform.position);

        float mult = (1.0f - closestDistance / boxLength) + 1.0f;
        glm::vec2 lateral = glm::vec2(0, (obPhysics.radius - obstacleLocalPos.y) * mult * 10.0f);
        glm::vec2 braking = glm::vec2((obPhysics.radius - obstacleLocalPos.x) * 10.0f, 0.0);

        return convertToWorld(head, glm::vec2(0.0f, 0.0f), lateral + braking);
    }

    return glm::vec2(0.0, 0.0);
}

glm::vec2 SteeringManager::obstacleAvoiding() {

}

glm::vec2 SteeringManager::wallAvoidance() {
    Transform& transform = m_registry->get<Transform>(m_owner);
    Physics& physics = m_registry->get<Physics>(m_owner);

    glm::vec2 head = safeNormalize(physics.velocity);

    if(head.x * head.x + head.y * head.y < 0.01f) {
        head = orientationToVec(transform.angle);
    }

    const static float lineLength = 80.0f;

    std::vector<Line2D> lines;
    for(int i = 0; i < 6; ++i) {
        Line2D line;
        line.start = glm::vec2(0.0f, 0.0f) + transform.position;
        line.end = lineLength * glm::vec2(std::cos(-M_PI * 0.375f + M_PI * 0.125f * i),
                                          std::sin(-M_PI * 0.375f + M_PI * 0.125f * i));
        line.end = convertToWorld(head, transform.position, line.end);

        lines.push_back(line);
    }

    float closestDistance = 32000.0f;
    float penetrationDepth = 0.0f;
    glm::vec2 wallNormal = glm::vec2(0.0f, 0.0f);

    auto wallsView = m_registry->view<Wall>();
    for(auto line : lines) {
        wallsView.each([&](entt::entity entity, Wall& wall){
            IntersectionResult result;
            if(testLineIntersection2D(wall.line, line, result)) {
                //std::cout << result.t1 << " " << result.t2 << "\n";
                if(result.t1 >= 0.0f && result.t1 <= 1.0f &&
                   result.t2 >= 0.0f && result.t2 <= 1.0f) {

                    float distance = glm::distance(transform.position, result.point);
                    if(distance < closestDistance) {
                        penetrationDepth = lineLength - distance;
                        wallNormal = wall.normal;
                        closestDistance = distance;
                    }

                }
            }


        });
    }

    if(closestDistance < lineLength) {
        return 2.5f * wallNormal * (penetrationDepth * penetrationDepth * penetrationDepth);
    }

    return glm::vec2(0.0f, 0.0f);
}

glm::vec2 SteeringManager::interpose(entt::entity targetA, entt::entity targetB) {
    Transform& playerTransform = m_registry->get<Transform>(m_owner);
    Physics& playerPhysics = m_registry->get<Physics>(m_owner);

    Transform& targetATransform = m_registry->get<Transform>(targetA);
    Physics& targetAPhysics = m_registry->get<Physics>(targetA);

    Transform& targetBTransform = m_registry->get<Transform>(targetB);
    Physics& targetBPhysics = m_registry->get<Physics>(targetB);

    glm::vec2 targetMidPoint = (targetATransform.position + targetBTransform.position) * 0.5f;
    float t = glm::distance(playerTransform.position, targetMidPoint) / playerPhysics.maxSpeed;

    glm::vec2 arrivePosition = ((targetATransform.position + targetAPhysics.velocity * t) +
                                (targetBTransform.position + targetBPhysics.velocity * t)) * 0.5f;

    return arrive(arrivePosition, FAST);
}


glm::vec2 SteeringManager::hide(entt::entity target) {

    Transform& playerTransform = m_registry->get<Transform>(m_owner);
    Transform& targetTransform = m_registry->get<Transform>(target);

    auto obstaclesView = m_registry->view<Obstacle, Transform, Physics>();

    float closestDistance = 32000.0f;
    glm::vec2 closestPoint = glm::vec2(0.0f, 0.0f);

    obstaclesView.each([&](entt::entity entity, Obstacle& obstacle, Transform& transform, Physics& physics){
        if(entity == m_owner || entity == target) {
            return;
        }

        glm::vec2 hidingSpot = findHidingSpot(transform.position, physics.radius, targetTransform.position);
        float distance = glm::distance(hidingSpot, playerTransform.position);
        if(distance < closestDistance) {
            closestDistance = distance;
            closestPoint = hidingSpot;
        }

    });

    if(closestDistance > 1000.0f) {
        return evade(target);
    }

    return arrive(closestPoint, FAST);
}

glm::vec2 SteeringManager::followPath() {
    glm::vec2 currentWaypoint = m_path.getCurrentWaypoint();
    if(m_path.isFinished()) {
        return arrive(currentWaypoint, FAST);
    }

    Transform& playerTransform = m_registry->get<Transform>(m_owner);
    if(glm::distance(playerTransform.position, currentWaypoint) < m_path.getWaypointThreshold()) {
        currentWaypoint = m_path.nextWaypoint();
    }

    return seek(currentWaypoint);
}

glm::vec2 SteeringManager::offsetPursuit(entt::entity target, glm::vec2 offset) {

    Transform& playerTransform = m_registry->get<Transform>(m_owner);
    Physics& playerPhysics = m_registry->get<Physics>(m_owner);

    Transform& targetTransform = m_registry->get<Transform>(target);
    Physics& targetPhysics = m_registry->get<Physics>(target);

    glm::vec2 head = targetPhysics.velocity;
    float length = glm::length(head);
    if(length < 0.01f) {
        head = orientationToVec(targetTransform.angle);
    } else {
        head *= (1.0f / length);
    }

    glm::vec2 globalOffset = convertToWorld(head, targetTransform.position, offset);
    float arriveTime = glm::distance(playerTransform.position, globalOffset) / playerPhysics.maxSpeed;

    return arrive(globalOffset + targetPhysics.velocity * arriveTime, FAST);
}

void SteeringManager::setPath(const WaypointsPath& path) {
    m_path = path;
}

glm::vec2 SteeringManager::findHidingSpot(glm::vec2 obstaclePosition,
                                          float obstacleRadius,
                                          glm::vec2 targetPosition) {
    const static float distanceFromObstacle = 30.0f;

    glm::vec2 direction = glm::normalize(obstaclePosition - targetPosition);

    return obstaclePosition + direction * (distanceFromObstacle + obstacleRadius);
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

