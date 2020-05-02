#ifndef PATH_H_INCLUDED
#define PATH_H_INCLUDED

/*
 * Class for different styles of paths for following path steering behaviors.
 *
 * The general idea behind paths are simple: some object has or take a path,
 * then looking for nearest point on the path from the current position. Then
 * It looks for some next point on the path and starts moving to the next point
 * with Seek behavior.
 *
 * Path uses a single parameter variable for describing a position.
 *
 * Whenever we need to convert a global position to parameter or vice versa
 * we use a getParam(vector) or getPosition(parameter) for achieving that.
 *
 * Different paths uses different approaches. For example, FormulaPath uses
 * some sort of formula to get a parameter - it takes a vector and translates
 * it with a early set formula (it's a 2 variables function:
 *                                                t = f(x,y) = f(vector))
 * ----------------------------------------------------------------------------
 * So in general we have an interface for translating coordinates to 1-D path`s
 * coordinates or vice versa. That's all!
 *
 */
class IPath {
public:
    virtual float getParam(glm::vec2 position) = 0;
    virtual glm::vec2 getPosition(float param) = 0;
};

#include <vector>

struct Segment {

    glm::vec2 getProjectedPoint(glm::vec2 worldPosition) const {

        // We solve two equations:
        // start + t*(end - start) = D
        // (end - start).(worldPosition - D) = 0
        //
        // Reference: stackoverflow.com/questions/10301001/perpendicular-on-a-line-segment-from-a-given-point

        float t = glm::dot(worldPosition - start, end - start);
        t /= glm::pow(glm::length(end - start), 2);

        return start + t * (end - start);
    }

    float getDistanceToPoint(glm::vec2 worldPosition) const {
        auto projected = getProjectedPoint(worldPosition);
        return glm::length(worldPosition - projected);
    }

    float length() const {
        return glm::length(start - end);
    }

    glm::vec2 direction() const {
        return glm::normalize(end - start);
    }

    glm::vec2 start;
    glm::vec2 end;
};

class SegmentedPath {
public:
    float getParam(glm::vec2 position) const {

        auto closestSegmentIdx = 0u;
        for(auto i = 0u; i < m_segments.size(); ++i) {
            if(m_segments[i].getDistanceToPoint(position) <
               m_segments[closestSegmentIdx].getDistanceToPoint(position)) {
                closestSegmentIdx = i;
            }
        }

        float t = 0.0f;
        if(closestSegmentIdx > 0) {
            for(auto i = 0u; i <= closestSegmentIdx - 1; i++) {
                t += m_segments[i].length();
            }
        }
        //std::cout << position.x << " " << position.y << std::endl;
        //std::cout << closestSegmentIdx << " " << m_segments[closestSegmentIdx].getProjectedPoint(position).x << " " << m_segments[closestSegmentIdx].getProjectedPoint(position).y << std::endl;
        t += glm::length(m_segments[closestSegmentIdx].getProjectedPoint(position) - m_segments[closestSegmentIdx].start);

        return t;
    }

    glm::vec2 getPosition(float param) const {
        auto segmentIndex = 0u;
        while(param > m_segments[segmentIndex].length()) {
            param -= m_segments[segmentIndex].length();
            segmentIndex++;
        }

        if(segmentIndex >= m_segments.size()) return glm::vec2(0.0f, 0.0f);

        return m_segments[segmentIndex].start + m_segments[segmentIndex].direction() * param;
    }

    void setPath(std::vector<Segment> segments) {
        m_segments = segments;
    }

private:
    std::vector<Segment> m_segments;
    float m_pathLength;
};

#endif // PATH_H_INCLUDED
