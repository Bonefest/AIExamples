#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include "entt.hpp"
#include "../Components/Components.h"
#include "../Events/Events.h"
#include "../GameData.h"

class ISystem {
public:
    virtual void enter(entt::registry& registry, entt::dispatcher& dispatcher) { }
    virtual void update(entt::registry& registry, entt::dispatcher& dispatcher, float delta) = 0;
};

class RenderingSystem: public ISystem {
public:
    RenderingSystem(SDL_Renderer* renderer): m_renderer(renderer) { }
    virtual void update(entt::registry& registry, entt::dispatcher& dispatcher, float delta) {
        auto renderableObjects = registry.view<Renderable, Transform>();
        renderableObjects.each([&](entt::entity object, Renderable& renderable, Transform& transform){
            if(renderable.texture != NULL) {
                SDL_Rect destRect;
                destRect.x = int(std::round(transform.position.x));
                destRect.y = int(std::round(transform.position.y));
                destRect.w = int(std::round(transform.size.x * transform.scale));
                destRect.h = int(std::round(transform.size.y * transform.scale));
                SDL_RenderCopyEx(m_renderer, renderable.texture, NULL, &destRect, transform.angle, NULL, SDL_FLIP_NONE);
            }
        });
    }

private:
    SDL_Renderer* m_renderer;
};

class PhysicsSystem: public ISystem {
public:
    virtual void update(entt::registry& registry, entt::dispatcher& dispatcher, float delta) {
        GameData& gameData = registry.ctx<GameData>();

        auto physicsObjects = registry.view<Transform, Kinematic>();
        physicsObjects.each([&](entt::entity object, Transform& transform, Kinematic& kinematic) {
            transform.position = wrapAround(transform.position + kinematic.velocity * delta,
                                            gameData.screenSize);

            transform.angle = glm::degrees(std::atan2(kinematic.velocity.y, kinematic.velocity.x));
        });
    }

private:
    glm::vec2 wrapAround(glm::vec2 position, glm::vec2 size) {
        glm::vec2 result(position);
        if(position.x > size.x + 64) result.x = -64;
        else if(position.x < -65) result.x = position.x + 63;

        if(position.y > size.y + 64) result.y = -64;
        else if(position.y < -65) result.y = size.y + 63;

        return result;
    }
};

class AISteeringSystem: public ISystem {
public:
    AISteeringSystem(): m_target(200.0f, 200.0f) { }

    virtual void enter(entt::registry& registry, entt::dispatcher& dispatcher) {
        dispatcher.sink<MouseEvent>().connect<&AISteeringSystem::onMouseEvent>(*this);
    }

    virtual void update(entt::registry& registry, entt::dispatcher& dispatcher, float delta) {
        auto aiobjects = registry.view<Kinematic, AI>();
        aiobjects.each([&](entt::entity object, Kinematic& kinematic, AI& ai) {
            glm::vec2 acceleration = ai.manager->seek(m_target);
            kinematic.velocity += acceleration * delta;
        });
    }

    void onMouseEvent(const MouseEvent& event) {
        if(event.event.type == SDL_MOUSEBUTTONDOWN)
            m_target = glm::vec2(event.event.x, event.event.y);
    }

private:
    glm::vec2 m_target;
};

#endif // SYSTEM_H_INCLUDED
