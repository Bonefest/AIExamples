#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include "entt.hpp"
#include "../Components/Components.h"
#include "../Events/Events.h"
#include "../GameData.h"
#include "../../Path.h"

#include "../helper.h"

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
                destRect.x = int(std::round(transform.position.x - transform.size.x * transform.scale * 0.5f));
                destRect.y = int(std::round(transform.position.y - transform.size.y * transform.scale * 0.5f));
                destRect.w = int(std::round(transform.size.x * transform.scale));
                destRect.h = int(std::round(transform.size.y * transform.scale));

                SDL_RenderCopyEx(m_renderer, renderable.texture, NULL, &destRect, transform.angle, NULL, SDL_FLIP_NONE);
            }
        });
    }

private:
    SDL_Renderer* m_renderer;
};

class ObstacleRenderingSystem: public ISystem {
public:
    ObstacleRenderingSystem(SDL_Renderer* renderer): m_renderer(renderer) { }

    virtual void update(entt::registry& registry, entt::dispatcher& dispatcher, float delta) {
        auto obstaclesView = registry.view<Obstacle, Physics, Transform>();
        for(auto obstacle: obstaclesView) {
            Physics& physics = registry.get<Physics>(obstacle);
            Transform& transform = registry.get<Transform>(obstacle);
            Obstacle& obst = registry.get<Obstacle>(obstacle);

            drawCircle(m_renderer, transform.position, physics.radius, obst.color, 32);
        }
    }

private:
    SDL_Renderer* m_renderer;
};

class ObstacleBoxRenderingSystem: public ISystem {
public:
    ObstacleBoxRenderingSystem(SDL_Renderer* renderer): m_renderer(renderer) { }

    virtual void update(entt::registry& registry, entt::dispatcher& dispatcher, float delta) {
        auto aiView = registry.view<Transform, Physics, AI>();
        aiView.each([&](entt::entity, Transform& transform, Physics& physics, AI& ai){
            glm::vec2 points[2];
            points[0] = transform.position;
            points[1] = safeNormalize(physics.velocity) * (physics.velocity.length() / physics.maxSpeed * 100.0f + 100.0f) + transform.position;

            SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);
            SDL_RenderDrawLine(m_renderer, int(points[0].x), int(points[0].y), int(points[1].x), int(points[1].y));
        });
    }

private:
    SDL_Renderer* m_renderer;
};

class PathRenderingSystem: public ISystem {
public:
    PathRenderingSystem(SDL_Renderer* renderer): m_renderer(renderer) { }

    virtual void update(entt::registry& registry, entt::dispatcher& dispatcher, float delta) {
        auto pathView = registry.view<Path>();
        for(auto path: pathView) {
            const Path& pathComponent = registry.get<Path>(path);
            if(!pathComponent.needRendering) continue;

            auto vertices = pathComponent.path->generateVertices();
            if(vertices.size() > 1) {
                for(auto i = 0u; i < vertices.size() - 1; ++i) {

                    auto start = vertices[i];
                    auto end = vertices[i + 1];

                    Uint8 r, g, b, a;
                    SDL_GetRenderDrawColor(m_renderer, &r, &g, &b, &a);
                    SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 255);
                    SDL_RenderDrawLine(m_renderer,
                                       std::round(start.x),
                                       std::round(start.y),
                                       std::round(end.x),
                                       std::round(end.y));

                    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
                }
            }
        }
    }

private:
    SDL_Renderer* m_renderer;
};

class WallRenderingSystem: public ISystem {
public:
    WallRenderingSystem(SDL_Renderer* renderer): m_renderer(renderer) { }

    virtual void update(entt::registry& registry, entt::dispatcher& dispatcher, float delta) {
        auto wallsView = registry.view<Wall>();
        wallsView.each([&](entt::entity entity, Wall& wall){

            Uint8 r, g, b, a;
            SDL_GetRenderDrawColor(m_renderer, &r, &g, &b, &a);
            SDL_SetRenderDrawColor(m_renderer, wall.color.r, wall.color.g, wall.color.b, 255);
            SDL_RenderDrawLine(m_renderer, std::round(wall.line.start.x), std::round(wall.line.start.y), std::round(wall.line.end.x), std::round(wall.line.end.y));


            glm::vec2 center = (wall.line.start + wall.line.end) * 0.5f;
            glm::vec2 normalEnd = center + wall.normal * 30.0f;


            SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 255);
            SDL_RenderDrawLine(m_renderer, std::round(center.x), std::round(center.y), std::round(normalEnd.x), std::round(normalEnd.y));


            SDL_SetRenderDrawColor(m_renderer, r, g, b, a);


        });
    }

private:
    SDL_Renderer*   m_renderer;

};

class PhysicsSystem: public ISystem {
public:
    virtual void update(entt::registry& registry, entt::dispatcher& dispatcher, float delta) {
        GameData& gameData = registry.ctx<GameData>();

        auto physicsObjects = registry.view<Transform, Physics>();
        physicsObjects.each([&](entt::entity object, Transform& transform, Physics& physics) {
            transform.position = wrapAround(transform.position + physics.velocity * delta,
                                            gameData.screenSize);

        });
    }

private:
    glm::vec2 wrapAround(glm::vec2 position, glm::vec2 size) {
        glm::vec2 result(position);
        if(position.x > size.x + 64) result.x = -64;
        else if(position.x < -65) result.x = size.x + 63;

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
        auto path = std::make_shared<SegmentedPath>();
        path->setPath({
                       {glm::vec2(0.0f, 10.0f), glm::vec2(100.0f, 10.0f)},
                       {glm::vec2(100.0f, 10.0f), glm::vec2(100.0f, 110.0f)},
                       {glm::vec2(100.0f, 110.0f), glm::vec2(200.0f, 110.0f)},
                       {glm::vec2(200.0f, 110.0f), glm::vec2(200.0f, 210.0f)},
                       {glm::vec2(200.0f, 210.0f), glm::vec2(100.0f, 310.0f)},
                       {glm::vec2(100.0f, 310.0f), glm::vec2(0.0f, 210.0f)},
                       {glm::vec2(0.0f, 210.0f), glm::vec2(0.0f, 10.0f)},
                      });

        m_path = registry.create();
        registry.assign<Path>(m_path, path, false);
    }

    virtual void update(entt::registry& registry, entt::dispatcher& dispatcher, float delta) {
        entt::entity player = entt::null;

        auto view = registry.view<Controllable>();
        for(auto entity : view) {
            player = entity;
            break;
        }

        auto aiobjects = registry.view<Transform, Physics, AI>();
        aiobjects.each([&](entt::entity object, Transform& transform, Physics& physics, AI& ai) {
            //ai.manager->target = player;

            glm::vec2 force = safeNormalize(ai.manager->calculate());
            //We temporary make result force maximal

            glm::vec2 acceleration = force * (1.0f / physics.mass) * physics.maxForce;
            physics.velocity = wrapVector(physics.velocity + acceleration * delta, physics.maxSpeed);

            transform.angle = vecToOrientation(physics.velocity);
        });
    }

    void onMouseEvent(const MouseEvent& event) {
        if(event.event.type == SDL_MOUSEBUTTONDOWN)
            m_target = glm::vec2(event.event.x, event.event.y);
    }

private:
    glm::vec2 m_target;
    entt::entity m_path;

};

#endif // SYSTEM_H_INCLUDED
