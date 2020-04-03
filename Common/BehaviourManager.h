#ifndef BEHAVIOURMANAGER_H_INCLUDED
#define BEHAVIOURMANAGER_H_INCLUDED

#include "glm/glm.hpp"
#include "Components/Components.h"

class BehaviourManager {
public:
    BehaviourManager(entt::registry& registry, entt::entity owner): m_rregistry(registry),
                                                                    m_owner(owner) { }
    glm::vec2 seek(glm::vec2 target) {
        Transform& ownerTransform = m_rregistry.get<Transform>(m_owner);
        Kinematic& ownerKinematic = m_rregistry.get<Kinematic>(m_owner);

        glm::vec2 desiredVelocity = glm::normalize(target - ownerTransform.position) * ownerKinematic.maxSpeed;
        return (desiredVelocity - ownerKinematic.velocity);
    }

    glm::vec2 flee(glm::vec2 target, float maxDistance) {
        Transform& ownerTransform = m_rregistry.get<Transform>(m_owner);
        Kinematic& ownerKinematic = m_rregistry.get<Kinematic>(m_owner);

        float distance = glm::length(ownerTransform.position - target);
        if(distance < maxDistance) {
            glm::vec2 desiredVelocity = glm::normalize(target - ownerTransform.position) * ownerKinematic.maxSpeed;
            return ownerKinematic.velocity - desiredVelocity;
        }

        return -ownerKinematic.velocity;
    }

    glm::vec2 simplifiedFlee(glm::vec2 target, float maxDistance) {
        Transform& ownerTransform = m_rregistry.get<Transform>(m_owner);
        Kinematic& ownerKinematic = m_rregistry.get<Kinematic>(m_owner);

        float distance = glm::length(ownerTransform.position - target);
        if(distance < maxDistance) {
            glm::vec2 desiredVelocity = glm::normalize(target - ownerTransform.position) * ownerKinematic.maxSpeed;
            return desiredVelocity;
        }

        return glm::vec2(0.0f, 0.0f);
    }

    glm::vec2 simplifiedArrive(glm::vec2 target, float radius) {
        Transform& ownerTransform = m_rregistry.get<Transform>(m_owner);
        Kinematic& ownerKinematic = m_rregistry.get<Kinematic>(m_owner);

        float distance = glm::length(ownerTransform.position - target);

        if(distance > radius) {
            float speed = std::min(distance / 0.25f, ownerKinematic.maxSpeed);
            glm::vec2 desiredVelocity = glm::normalize(target - ownerTransform.position)  * speed;
            return desiredVelocity;
        }

        return glm::vec2(0.0f, 0.0f);
    }

private:

    entt::registry& m_rregistry;
    entt::entity m_owner;
};

struct SteeringOutput {
    glm::vec2 linear;
    float angular;
};



#endif // BEHAVIOURMANAGER_H_INCLUDED
