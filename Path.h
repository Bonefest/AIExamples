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
    virtual float getParam(glm::vec2 position) const = 0;
    virtual glm::vec2 getPosition(float param) const = 0;

    virtual std::vector<glm::vec2> generateVertices() const {
        return std::vector<glm::vec2>{};
    }
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

class SegmentedPath: public IPath {
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

    std::vector<glm::vec2> generateVertices() const {
        if(m_segments.empty()) return std::vector<glm::vec2>{};

        std::vector<glm::vec2> result;
        for(auto segment: m_segments) {
            result.push_back(segment.start);
            result.push_back(segment.end);
        }

        result.push_back(m_segments.back().end);

        return result;
    }

private:
    std::vector<Segment> m_segments;
    float m_pathLength;
};

using Waypoints = std::vector<glm::vec2>;

class WaypointsPath {
public:
    WaypointsPath(): m_threshold(0.0f), m_looped(false) { }
    WaypointsPath(const Waypoints& waypoints, float threshold, bool looped): m_waypoints(waypoints),
                                                                             m_currentWaypoint(0),
                                                                             m_threshold(threshold),
                                                                             m_looped(looped) { }

    glm::vec2 getCurrentWaypoint() const {

        return (m_waypoints.empty()) ? glm::vec2(0.0f, 0.0f) : m_waypoints[m_currentWaypoint];
    }

    glm::vec2 nextWaypoint() {
        if(m_currentWaypoint < m_waypoints.size() - 1) {
            m_currentWaypoint++;
        } else if(m_looped) {
            m_currentWaypoint = 0;
        }

        return m_waypoints[m_currentWaypoint];
    }

    bool isFinished() const {

        return (m_waypoints.empty() || (m_currentWaypoint == m_waypoints.size() - 1 && !m_looped));
    }

    void setWaypoints(const Waypoints& waypoints) {
        m_waypoints = waypoints;
    }

    void setThreshold(float threshold) {
        m_threshold = threshold;
    }

    float getWaypointThreshold() const {
        return m_threshold;
    }



private:
    Waypoints       m_waypoints;
    std::size_t     m_currentWaypoint;
    float           m_threshold;
    bool            m_looped;

};

#endif // PATH_H_INCLUDED
