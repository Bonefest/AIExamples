#ifndef SYSTEMSMANAGER_H_INCLUDED
#define SYSTEMSMANAGER_H_INCLUDED

#include <memory>
#include <vector>

using std::shared_ptr;
using std::vector;

#include "Systems/System.h"

class SystemsManager {
public:
    void addSystem(shared_ptr<ISystem> system) {
        m_systems.push_back(system);
    }

    void update(float delta) {
        for(auto system : m_systems) {
            system->update(m_registry, m_dispatcher, delta);
        }
    }
private:
    entt::registry m_registry;
    entt::dispatcher m_dispatcher;

    vector<shared_ptr<ISystem>> m_systems;
};



#endif // SYSTEMSMANAGER_H_INCLUDED
