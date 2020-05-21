#include "helper.h"

#include <iostream>
glm::vec2 calculatePerp(glm::vec2 vec) {
    return glm::vec2(-vec.y, vec.x);
}

void drawCircle(SDL_Renderer* renderer, glm::vec2 position, float radius, SDL_Color color, int segments) {
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    int xcenter = std::round(position.x), ycenter = std::round(position.y);
    int xprv = xcenter + radius, yprv = ycenter;
    float step = 2 * M_PI / segments;

    for(int i = 1;i < segments; ++i) {
        int xcur = xcenter + std::cos(i * step) * radius, ycur = ycenter + std::sin(i * step) * radius;
        SDL_RenderDrawLine(renderer, xprv, yprv, xcur, ycur);

        xprv = xcur; yprv = ycur;
    }

    SDL_RenderDrawLine(renderer, xprv, yprv, xcenter + radius, ycenter);

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

glm::vec2 convertToLocal(glm::vec2 ownerHeadingVec, glm::vec2 ownerPos, glm::vec2 targetPos) {
    glm::vec2 ownerPerp = glm::normalize(calculatePerp(ownerHeadingVec));
    glm::mat3 converter = glm::mat3(1.0f);
    converter[0] = glm::vec3(ownerHeadingVec, 0.0f);    //First column:  I
    converter[1] = glm::vec3(ownerPerp, 0.0f);          //Second column: J
    converter[2] = glm::vec3(ownerPos, 1.0f);           //Third column:  Translate

    converter = glm::inverse(converter);
    return glm::vec2(converter * glm::vec3(targetPos, 1.0f));
}
