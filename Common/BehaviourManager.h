#ifndef BEHAVIOURMANAGER_H_INCLUDED
#define BEHAVIOURMANAGER_H_INCLUDED

#include "glm/glm.hpp"
#include "Components/Components.h"


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

        Output output;

        if(distance < maxRadius) {
            output.acceleration = glm::normalize(transform.position - target) * kinematic.maxAcceleration;
        } else {
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

        float angleDistance = abs(std::fmod(target - transform.angle, 360.0f));
        printf("%f %f\n", target, angleDistance);

        if(angleDistance < stopRadius)
            return output;

        float speed = kinematic.maxAngularSpeed;
        if(angleDistance < angleRadius)
            speed = angleDistance / angleRadius;

        float direction = (target - transform.angle) / abs(target - transform.angle);

        output.angular = speed * direction - kinematic.angularSpeed;
        output.angular /= 0.01f;

        if(abs(output.angular) > kinematic.maxAngularSpeed) {
            output.angular = output.angular / abs(output.angular) * kinematic.maxAngularSpeed;
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
