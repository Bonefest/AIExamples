#ifndef BEHAVIOURMANAGER_H_INCLUDED
#define BEHAVIOURMANAGER_H_INCLUDED

#include "glm/glm.hpp"
#include "Components/Components.h"
#include "../Path.h"


struct SteeringOutput {
    glm::vec2 linear;
    float angular;
};

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

namespace KinematicSteeringBehaviours {
    struct Output {
        glm::vec2 velocity;
        float orientation;
    };

    Output seek(entt::registry& registry, entt::entity owner, glm::vec2 target) {
        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);

        Output output;
        output.velocity = glm::normalize(target - transform.position) * kinematic.maxSpeed;
        output.orientation = vecToOrientation(output.velocity);

        return output;
    }

    Output flee(entt::registry& registry, entt::entity owner, glm::vec2 target, float maxDistance) {
        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);

        float distance = glm::length(target - transform.position);

        Output output{glm::vec2(0.0f, 0.0f), transform.angle};
        if(distance < maxDistance) {
            output.velocity = glm::normalize(transform.position - target) * kinematic.maxSpeed;
            output.orientation = vecToOrientation(output.velocity);
        }

        return output;
    }

    Output arrive(entt::registry& registry, entt::entity owner, glm::vec2 target, float maxRadius) {
        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);

        Output output{glm::vec2(0.0f, 0.0f), transform.angle};

        float distance = glm::length(target - transform.position);
        if(distance > maxRadius) {
            float speed = std::min(distance / 0.25f, kinematic.maxSpeed);

            glm::vec2 desiredVelocity = glm::normalize(target - transform.position) * speed;
            output.velocity = desiredVelocity;
            output.orientation = vecToOrientation(desiredVelocity);
        }

        return output;
    }

    Output wandering(entt::registry& registry, entt::entity owner, float maxOrientation) {
        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);

        Output output;
        output.velocity = orientationToVec(glm::radians(transform.angle)) * kinematic.maxSpeed;
        output.orientation = (drand48() - drand48()) * maxOrientation;

        return output;
    }
}

namespace ksb = KinematicSteeringBehaviours;

namespace SteeringBehaviours {
    struct Output {
        glm::vec2 acceleration;
        float angular;
    };

    Output seek(entt::registry& registry, entt::entity owner, glm::vec2 target) {
        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);

        Output output;
        output.acceleration = glm::normalize(target - transform.position) * kinematic.maxAcceleration;

        return output;
    }

    Output flee(entt::registry& registry, entt::entity owner, glm::vec2 target, float maxRadius) {
        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);

        float distance = glm::length(target - transform.position);

        Output output{glm::vec2(0.0f, 0.0f), 0.0f};

        if(distance < maxRadius) {
            output.acceleration = glm::normalize(transform.position - target) * kinematic.maxAcceleration;
        } else if(glm::length(kinematic.velocity) > 0.01f) {
            output.acceleration = -glm::normalize(kinematic.velocity) * kinematic.maxAcceleration;
        }

        return output;
    }

    //OK parameters are: 1.0f, 300.0f
    Output arrive(entt::registry& registry, entt::entity owner, glm::vec2 target, float stopRadius, float velocityRadius) {
        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);

        Output output{glm::vec2(0.0f), 0.0f};

        glm::vec2 direction = target - transform.position;
        float distance = glm::length(direction);

        if(distance < stopRadius) return output;

        float speed = kinematic.maxSpeed;
        if(distance < velocityRadius)
            speed *= distance / velocityRadius;

        output.acceleration = glm::normalize(direction) * speed - kinematic.velocity;
        output.acceleration /= 0.1f;

        if(glm::length(output.acceleration) > kinematic.maxAcceleration)
            output.acceleration = glm::normalize(output.acceleration) * kinematic.maxAcceleration;

        return output;
    }

    //It's better to imagine distance between angles like a line not a circular path - then it's much easier to apply arrive method
    Output align(entt::registry& registry, entt::entity owner, float target, float stopRadius, float angleRadius) {
        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);

        Output output{glm::vec2(0.0f), 0.0f};

        float angle = transform.angle;

        float angleDistance = 180.0f - std::fabs(std::fmod(std::fabs(target - angle), 360.0f) - 180.0f);    //shortest path

        target = std::fmod(std::fmod(target, 360.0f) + 360.0f, 360.0f);
        angle = std::fmod(std::fmod(transform.angle, 360.0f) + 360.0f, 360.0f);

        float direction = (target - angle) / std::fabs(target - angle);
        if(std::fabs(std::fabs(target - angle) - angleDistance) > 0.1f)
            direction = -direction;



        if(angleDistance < stopRadius)
            return output;

        float speed = kinematic.maxAngularSpeed;
        if(angleDistance < angleRadius)
            speed *= angleDistance / angleRadius;

        output.angular = speed * direction - kinematic.angularSpeed;
        output.angular /= 0.01f;

        if(abs(output.angular) > kinematic.maxAngularSpeed) {
            output.angular = output.angular / abs(output.angular) * kinematic.maxAngularSpeed;
        }

        return output;
    }

    Output velocityMatch(entt::registry& registry, entt::entity owner, glm::vec2 targetVelocity) {
        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);

        Output output{glm::vec2(0.0f), 0.0f};
        output.acceleration = targetVelocity - kinematic.velocity;
        output.acceleration /= 0.1f;

        if(glm::length(output.acceleration) > kinematic.maxAcceleration)
            output.acceleration = glm::normalize(output.acceleration) * kinematic.maxAcceleration;

        return output;
    }

    Output simplePursue(entt::registry& registry, entt::entity owner, entt::entity target) {
        Transform& transform = registry.get<Transform>(target);

        return seek(registry, owner, transform.position);
    }

    Output pursue(entt::registry& registry, entt::entity owner, entt::entity target, float maxTime) {
        Transform& ownerTransform = registry.get<Transform>(owner);
        Kinematic& ownerKinematic = registry.get<Kinematic>(owner);

        Transform& transform = registry.get<Transform>(target);
        Kinematic& kinematic = registry.get<Kinematic>(target);

        float time = glm::length(transform.position - ownerTransform.position) / ownerKinematic.maxSpeed;
        time = std::min(time, maxTime);

        glm::vec2 targetPos = kinematic.velocity * time + transform.position;

        return seek(registry, owner, targetPos);
    }

    Output evade(entt::registry& registry, entt::entity owner, entt::entity target, float maxTime) {
        Transform& ownerTransform = registry.get<Transform>(owner);
        Kinematic& ownerKinematic = registry.get<Kinematic>(owner);

        Transform& transform = registry.get<Transform>(target);
        Kinematic& kinematic = registry.get<Kinematic>(target);

        float time = glm::length(transform.position - ownerTransform.position) / ownerKinematic.maxSpeed;
        time = std::min(time, maxTime);

        glm::vec2 targetPos = kinematic.velocity * time + transform.position;

        return flee(registry, owner, targetPos, 300.0f);
    }

    Output face(entt::registry& registry, entt::entity owner, entt::entity target) {
        Transform& ownerTransform = registry.get<Transform>(owner);

        Transform& transform = registry.get<Transform>(target);

        glm::vec2 vec = glm::normalize(transform.position - ownerTransform.position);
        return align(registry, owner, vecToOrientation(vec), 0.1f, 20.0f);
    }

    Output lookAtMovingDirection(entt::registry& registry, entt::entity owner) {
        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);

        float targetAngle = transform.angle;
        if(glm::length(kinematic.velocity) > 0.01f)
            targetAngle = vecToOrientation(kinematic.velocity);

        return align(registry, owner, targetAngle, 0.1f, 50.0f);
    }

    Output wander_dirty(entt::registry& registry, entt::entity owner, float wanderRadius, float wanderOffset, float wanderRate) {
        static float wanderAngle = 0.0f;

        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);

        wanderAngle += (drand48() - drand48()) * wanderRate;

        float targetAngle = transform.angle + wanderAngle;
        std::cout << targetAngle << std::endl;

        glm::vec2 target = transform.position + orientationToVec(glm::radians(transform.angle)) * wanderOffset;
        target = target + orientationToVec(glm::radians(targetAngle)) * wanderRadius;

        Output output = align(registry, owner, vecToOrientation(glm::normalize(target - transform.position)), 0.1f, 10.0f);
        output.acceleration = orientationToVec(glm::radians(transform.angle)) * kinematic.maxAcceleration;

        return output;
    }

    Output followPath(entt::registry& registry, entt::entity owner, entt::entity epath) {
        if(!registry.valid(epath) || !registry.has<Path>(epath)) return Output{};

        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);
        const auto& path = registry.get<Path>(epath).path;

        glm::vec2 target = path->getPosition(path->getParam(transform.position) + 80.0f);

        return seek(registry, owner, target);
    }

    Output followPredictedPath(entt::registry& registry, entt::entity owner, float time, entt::entity epath) {
        if(!registry.valid(epath) || !registry.has<Path>(epath)) return Output{};

        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);
        const auto& path = registry.get<Path>(epath).path;

        glm::vec2 velocity = kinematic.velocity;
        if(glm::length(velocity) > 0.01f)
            velocity = glm::normalize(velocity);

        glm::vec2 target = transform.position + velocity * kinematic.maxSpeed * time;
        target = path->getPosition(path->getParam(target) + 80.0f);
        return seek(registry, owner, target);
    }

    Output separation(entt::registry& registry, entt::entity owner, float k, float threshold, std::vector<entt::entity> targets) {
        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);

        Output output;
        for(auto target: targets) {
            if(auto targetTransform = registry.try_get<Transform>(target); targetTransform) {
                glm::vec2 direction = transform.position - targetTransform->position;
                float distance = glm::length(direction);
                if(distance < threshold) {
                    float strength = std::min(k / (distance * distance), kinematic.maxAcceleration);

                    output.acceleration += direction / distance * strength;
                }
            }
        }

        return output;
    }

    Output collisionAvoidance(entt::registry& registry, entt::entity owner, float radius, std::vector<entt::entity> targets) {
        Transform& transform = registry.get<Transform>(owner);
        Kinematic& kinematic = registry.get<Kinematic>(owner);

        entt::entity closestEntity = entt::null;
        float max_time = 5.0f;
        float shortestTime = max_time;

        for(auto target: targets) {
            if(!registry.valid(target) ||
               !registry.has<Kinematic>(target) ||
               !registry.has<Transform>(target)) continue;

            Transform& targetTransform = registry.get<Transform>(target);
            Kinematic& targetKinematic = registry.get<Kinematic>(target);

            glm::vec2 rp = targetTransform.position - transform.position;
            glm::vec2 rv = targetKinematic.velocity - kinematic.velocity;

            float t = glm::dot(rp, rv) / glm::pow(glm::length(rv), 2);
            std::cout <<t << std::endl;

            if(t > max_time || t < 0.0f) continue;

            if(t < shortestTime) {
                shortestTime = t;
                closestEntity = target;
            }

        }


        Output output{};
        if(registry.valid(closestEntity)) {
            Transform& targetTransform = registry.get<Transform>(closestEntity);
            Kinematic& targetKinematic = registry.get<Kinematic>(closestEntity);

            glm::vec2 futurePos = transform.position + kinematic.velocity * shortestTime;
            glm::vec2 targetFuturePos = targetTransform.position + targetKinematic.velocity * shortestTime;

            glm::vec2 relativePos = futurePos - targetFuturePos;

            if(glm::length(relativePos) < radius) {
                output.acceleration = glm::normalize(relativePos) * kinematic.maxAcceleration;
            }
        }

        return output;
    }
}

namespace sb = SteeringBehaviours;

class BehaviourManager {
public:
    BehaviourManager(entt::registry& registry, entt::entity owner): m_rregistry(registry),
                                                                    m_owner(owner) { }


private:

    entt::registry& m_rregistry;
    entt::entity m_owner;
};



#endif // BEHAVIOURMANAGER_H_INCLUDED
