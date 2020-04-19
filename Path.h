#ifndef PATH_H_INCLUDED
#define PATH_H_INCLUDED

class IPath {
public:
    virtual float getParam(glm::vec2 position) = 0;
    virtual glm::vec2 getPosition(float param) = 0;
};

#include <vector>

class SegmentedPath {
public:
    float getParam(glm::vec2 position) {
        if(m_vertices.size() < 2) return 0.0f;

        std::size_t nearestVertexIndex = 0;
        for(std::size_t i = 0; i < m_vertices.size(); ++i) {
            if(glm::distance(position, m_vertices[i]) < glm::distance(position, m_vertices[nearestVertexIndex]))
                nearestVertexIndex = i;
        }

        glm::vec2 directionToTarget = glm::normalize(position - m_vertices[nearestVertexIndex]);
        std::size_t segmentIndex = nearestVertexIndex;
        float minAngle = 360.0f;

        if(nearestVertexIndex != 0) {
            segmentIndex = nearestVertexIndex - 1;
            glm::vec2 segment = m_vertices[nearestVertexIndex - 1] - m_vertices[nearestVertexIndex];
            minAngle = std::acos(glm::dot(glm::normalize(segment), directionToTarget));
        }

        if(nearestVertexIndex != m_vertices.size() - 1) {
            glm::vec2 segment = m_vertices[nearestVertexIndex + 1] - m_vertices[nearestVertexIndex];
            float angle = std::acos(glm::dot(glm::normalize(segment), directionToTarget));

            if(angle < minAngle) {
                segmentIndex = nearestVertexIndex + 1;
            }
        }

        glm::vec2 segment = m_vertices[segmentIndex] - m_vertices[nearestVertexIndex];
        glm::vec2 projectedPoint = glm::dot(segment, directionToTarget) * glm::normalize(segment);

        //finding parameter...
    }

    glm::vec2 getPosition(float param);

    void setPath(std::vector<glm::vec2> vertices) {
        m_vertices = vertices;
    }

private:
    std::vector<glm::vec2> m_vertices;
    float m_pathLength;
};

#endif // PATH_H_INCLUDED
