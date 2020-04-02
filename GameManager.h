#ifndef GAMEMANAGER_H_INCLUDED
#define GAMEMANAGER_H_INCLUDED


#include <iostream>
#include "SystemsManager.h"
#include "Systems/System.h"


using namespace std;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

class GameManager {
public:
    GameManager(): m_pwindow(NULL), m_prenderer(NULL), m_fpsLimit(0), m_fps(0) { }
    ~GameManager() {
        if(m_pwindow != NULL) SDL_DestroyWindow(m_pwindow);
        SDL_Quit();
    }

    bool init() {
        if(!initSDL(m_pwindow, "Examples", SCREEN_WIDTH, SCREEN_HEIGHT)) return false;

        m_prenderer = SDL_CreateRenderer(m_pwindow, -1, SDL_RENDERER_ACCELERATED);

        return true;
    }

    void run() {
        uint32_t lastTime, currentTime, elapsedTime, fps;
        lastTime = currentTime = elapsedTime = fps = 0;

        bool finished = false;
        while(!finished) {
            //physics update should be unlimited!
            lastTime = currentTime;
            currentTime = SDL_GetTicks();

            if(elapsedTime < 1000) {
                if(fps < m_fpsLimit) {
                    pollEvents();
                    update(float(currentTime - lastTime) / 1000.0f);

                    fps++;
                }

                elapsedTime += (currentTime - lastTime);
            } else {
                m_fps = fps;
                elapsedTime = 0;
            }
        }
        currentTime = SDL_GetTicks();

    }

    void update(float delta) {
        m_systemsManager.update(delta);
    }

    void pollEvents() {
        SDL_Event event;
        while(SDL_PollEvent(&event) != 0) continue;
    }

    uint32_t getFPS() const { return m_fps; }
private:
    bool initSDL(SDL_Window* window,
                 const char* title,
                 int screenWidth,
                 int screenHeight,
                 int screenPosX = SDL_WINDOWPOS_UNDEFINED,
                 int screenPosY = SDL_WINDOWPOS_UNDEFINED) {

        if(SDL_Init(SDL_INIT_VIDEO) < 0) {
            showLastError("Unable to init SDL");
            return false;
        } else {
            window = SDL_CreateWindow(title, screenPosX, screenPosY, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
            if(window == NULL) {
                showLastError("Unable to create window");
                return false;
            }

            return true;

        }

    }

    void showLastError(const char* message) {
        printf("%s: %s\n", message, SDL_GetError());
    }




    SystemsManager m_systemsManager;
    SDL_Window* m_pwindow;
    SDL_Renderer* m_prenderer;

    uint32_t m_fpsLimit;
    uint32_t m_fps;
};



#endif // GAMEMANAGER_H_INCLUDED
