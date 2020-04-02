#include <iostream>
#include "SDL2/SDL.h"

#include "glm/glm.hpp"
#include "GameManager.h"

int main() {
    GameManager manager;
    if(manager.init()) {
        manager.run();
    }
    return 0;
}

