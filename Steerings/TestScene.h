#ifndef TESTSCENE_H_INCLUDED
#define TESTSCENE_H_INCLUDED

#include "GameManager.h"
#include "TextureManager.h"

class TestScene: public GameManager {
public:
    virtual bool init(const string& programName, int width, int height) {
        if(!GameManager::init(programName, width, height)) return false;

        TextureManager::getInstance().setRenderer(m_prenderer);

        initContext();
        initSystems();
        initEntities();

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

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
        SDL_Texture* texture = TextureManager::getInstance().loadTexture("Resources/Pointer.png");
        int textureWidth = 0, textureHeight = 0;
        SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);

        entt::registry& registry = m_systemsManager.getRegistry();
        entt::entity shipEntity = registry.create();

        registry.assign<Renderable>(shipEntity, texture);
        registry.assign<Transform>(shipEntity,
                                   glm::vec2(320.0f, 240.0f),
                                   glm::vec2(textureWidth, textureHeight),
                                   1.0f,
                                   0.0f);
        registry.assign<Kinematic>(shipEntity, 400.0f);
        auto bmanager = make_shared<BehaviourManager>(m_systemsManager.getRegistry(), shipEntity);
        registry.assign<AI>(shipEntity, bmanager);
    }

    virtual void update(float delta) {
        SDL_SetRenderDrawColor(m_prenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(m_prenderer);

        m_systemsManager.update(delta);

        SDL_RenderPresent(m_prenderer);
    }
};


#endif // TESTSCENE_H_INCLUDED
