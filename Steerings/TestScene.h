#ifndef TESTSCENE_H_INCLUDED
#define TESTSCENE_H_INCLUDED

#include "GameManager.h"
#include "TextureManager.h"

entt::entity createDrawableEntity(entt::registry& registry, const string& textureName, glm::vec2 position) {
    SDL_Texture* texture = TextureManager::getInstance().loadTexture(textureName);
    int tw, th;
    SDL_QueryTexture(texture, NULL, NULL, &tw, &th);

    entt::entity entity = registry.create();

    registry.assign<Renderable>(entity, texture);
    registry.assign<Transform>(entity, position,  glm::vec2(tw, th), 1.0f, 0.0f);

    return entity;
}

class TestScene: public GameManager {
public:
    virtual bool init(const string& programName, int width, int height) {
        if(!GameManager::init(programName, width, height)) return false;

        TextureManager::getInstance().setRenderer(m_prenderer);

        initContext();
        initSystems();
        initEntities();

        m_systemsManager.getDispatcher().sink<MouseEvent>().connect<&TestScene::onMouseEvent>(*this);

        return true;
    }


    void initContext() {
        auto& registry = m_systemsManager.getRegistry();

        registry.set<GameData>();
        GameData& gameData = registry.ctx<GameData>();

        gameData.screenSize = glm::vec2(m_screenWidth, m_screenHeight);

    }

    void initSystems() {
        m_systemsManager.addSystem(make_shared<RenderingSystem>(m_prenderer));
        m_systemsManager.addSystem(make_shared<PhysicsSystem>());
        m_systemsManager.addSystem(make_shared<AISteeringSystem>());
    }

    void initEntities() {
        entt::registry& registry = m_systemsManager.getRegistry();

        entt::entity shipEntity = createDrawableEntity(registry, "Resources/Pointer.png", glm::vec2(320, 240));
        registry.assign<Kinematic>(shipEntity, 500.0f, 500.0f, 100.0f);
        auto bmanager = make_shared<BehaviourManager>(m_systemsManager.getRegistry(), shipEntity);
        registry.assign<AI>(shipEntity, bmanager);


        m_markEntity = createDrawableEntity(registry, "Resources/Mark.png", glm::vec2(200, 200));
    }

    virtual void update(float delta) {
        SDL_SetRenderDrawColor(m_prenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(m_prenderer);

        m_systemsManager.update(delta);

        SDL_RenderPresent(m_prenderer);
    }

    void onMouseEvent(const MouseEvent& event) {
        Transform& transform = m_systemsManager.getRegistry().get<Transform>(m_markEntity);
        transform.position = glm::vec2(event.event.x, event.event.y);
    }

private:
    entt::entity m_markEntity;
};


#endif // TESTSCENE_H_INCLUDED
