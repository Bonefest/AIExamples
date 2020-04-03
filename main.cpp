#include <iostream>
#include "SDL2/SDL.h"

#include "Steerings/TestScene.h"

int main() {
    TestScene scene;
    if(scene.init("TestScene", 640, 480)) {
        scene.run();
    }
    return 0;
}

