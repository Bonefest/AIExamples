#ifndef GAMEMANAGER_H_INCLUDED
#define GAMEMANAGER_H_INCLUDED


#include <iostream>
#include <chrono>

#include "SystemsManager.h"
#include "Systems/System.h"
#include "Events/Events.h"

#include "GameData.h"

using namespace std;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

class GameManager {
public:
    GameManager(): m_pwindow(NULL), m_prenderer(NULL), m_fpsLimit(9999), m_fps(0) { }
    ~GameManager() {

        for(auto texture : m_vtextures) {
            SDL_DestroyTexture(texture);
        }

        SDL_DestroyRenderer(m_prenderer);

        if(m_pwindow != NULL) SDL_DestroyWindow(m_pwindow);
        SDL_Quit();
    }

    virtual bool init(const string& programName, int width, int height) {
        if(!initSDL(m_pwindow, programName.c_str(), width, height)) return false;

        return true;
    }


    void run() {
        std::chrono::time_point<std::chrono::system_clock> lastTime, currentTime;
        currentTime = lastTime = std::chrono::system_clock::now();
        double elapsedTime = 0.0;

        uint32_t fps = 0;

        std::chrono::duration<double> diff;

        m_gameFinished = false;
        while(!m_gameFinished) {
            //physics update should be unlimited!
            lastTime = currentTime;
            currentTime = std::chrono::system_clock::now();
            diff = currentTime - lastTime;
            double delta = diff.count();

            if(elapsedTime < 1.0) {
                if(fps < m_fpsLimit) {
                    pollEvents();
                    update(delta);

                    fps++;
                }

                elapsedTime += delta;
            } else {
                m_fps = fps;
                elapsedTime = 0;
                fps = 0;
            }
        }
    }

    virtual void update(float delta) { }

    virtual void pollEvents() {
        SDL_Event event;
        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_QUIT) {
                m_gameFinished = true;
            } else if(event.type == SDL_MOUSEBUTTONDOWN ||
                      event.type == SDL_MOUSEBUTTONUP) {
                m_systemsManager.getDispatcher().trigger<MouseEvent>(event.button);
            }
        }
    }

    uint32_t getFPS() const { return m_fps; }

protected:
    SystemsManager m_systemsManager;
    SDL_Window* m_pwindow;
    SDL_Renderer* m_prenderer;

    int m_screenWidth;
    int m_screenHeight;
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
            m_screenWidth = screenWidth;
            m_screenHeight = screenHeight;

            window = SDL_CreateWindow(title, screenPosX, screenPosY, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
            if(window == NULL) {
                showLastError("Unable to create window");
                return false;
            }

            m_prenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if(m_prenderer == NULL) return false;

            return true;

        }

    }

    void showLastError(const char* message) {
        printf("%s: %s\n", message, SDL_GetError());
    }


    bool m_gameFinished;

    uint32_t m_fpsLimit;
    uint32_t m_fps;

    vector<SDL_Texture*> m_vtextures;
};



#endif // GAMEMANAGER_H_INCLUDED
