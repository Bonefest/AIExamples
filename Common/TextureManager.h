#ifndef TEXTUREMANAGER_H_INCLUDED
#define TEXTUREMANAGER_H_INCLUDED

#include <map>
#include <string>

using std::map;
using std::string;

#include "SDL2/SDL_image.h"

class TextureManager {
public:
    ~TextureManager() {
        for(auto texturePair : m_loadedTextures) {
            SDL_DestroyTexture(texturePair.second);
        }
    }

    static TextureManager& getInstance() {
        static TextureManager manager;
        return manager;
    }

    void setRenderer(SDL_Renderer* renderer) {
        m_prenderer = renderer;
    }

    SDL_Texture* loadTexture(const string& fileName, bool forceLoad = false) {
        SDL_Surface* surface = IMG_Load(fileName.c_str());
        SDL_Texture* texture = SDL_CreateTextureFromSurface(m_prenderer, surface);
        SDL_FreeSurface(surface);

        auto textureIter = m_loadedTextures.find(fileName);
        if(forceLoad && textureIter != m_loadedTextures.end()) {
            SDL_DestroyTexture(textureIter->second);
            textureIter->second = texture;
        } else if(textureIter != m_loadedTextures.end()) {
            texture = textureIter->second;
        } else {
            m_loadedTextures[fileName] = texture;
        }

        return texture;
    }
private:
    TextureManager() { }
    TextureManager(const TextureManager&);
    TextureManager& operator=(const TextureManager&);

    map<string, SDL_Texture*> m_loadedTextures;
    SDL_Renderer* m_prenderer;
};

#endif // TEXTUREMANAGER_H_INCLUDED
