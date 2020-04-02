#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include "entt.hpp"
#include "../Components/Components.h"

class ISystem {
public:
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
                destRect.w = int(std::round(transform.size.x));
                destRect.h = int(std::round(transform.size.y));
                SDL_RenderCopyEx(m_renderer, renderable.texture, NULL, &destRect, transform.angle, NULL, SDL_FLIP_NONE);
            }
        });
    }

private:
    SDL_Renderer* m_renderer;
};

#endif // SYSTEM_H_INCLUDED
