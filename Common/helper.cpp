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
    glm::vec2 head = glm::normalize(ownerHeadingVec);
    glm::vec2 perp = glm::normalize(calculatePerp(ownerHeadingVec));
    glm::vec2 result = targetPos - ownerPos;

    // Unoptimized way!
    glm::mat2x2 mat;
    mat[0][0] = head.x; mat[0][1] = head.y;
    mat[1][0] = perp.x; mat[1][1] = perp.y;

    mat = glm::inverse(mat);

    return mat * result;
}


glm::vec2 convertToWorld(glm::vec2 ownerHeadingVec, glm::vec2 ownerPos, glm::vec2 targetPos) {
    glm::vec2 head = glm::normalize(ownerHeadingVec);
    glm::vec2 side = glm::normalize(calculatePerp(ownerHeadingVec));

    // Affine transform
    glm::vec2 result = head * targetPos.x + side * targetPos.y + ownerPos;

    return result;
}

bool testLineIntersection2D(Line2D lineA, Line2D lineB, IntersectionResult& result) {
    glm::vec2 d1 = lineA.end - lineA.start;
    glm::vec2 d2 = lineB.end - lineB.start;
    glm::vec2 r  = lineA.start - lineB.start;

    float a = glm::dot(d1, d1);
    float b = glm::dot(d1, d2);
    float e = glm::dot(d2, d2);

    float d = a * e - b * b;
    if(abs(d) < 0.001) {
        return false;
    }

    float c = glm::dot(d1, r);
    float f = glm::dot(d2, r);

    result.t1 = (b*f - c*e) / d;
    result.t2 = (a*f - b*c) / d;
    result.point = lineA.start + d1 * result.t1;

    return true;
}
