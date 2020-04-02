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
        Physics& ownerPhysics = m_rregistry.get<Physics>(m_owner);

        glm::vec2 desiredVelocity = glm::normalize(target - ownerTransform.position) * ownerPhysics.maxSpeed;
        return (desiredVelocity - ownerPhysics.velocity);
    }

private:

    entt::registry& m_rregistry;
    entt::entity m_owner;
};

#endif // BEHAVIOURMANAGER_H_INCLUDED
