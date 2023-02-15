#ifndef LINE2D_HPP
#define LINE2D_HPP

#include <glm/glm.hpp>

enum Direction {
    TOP,
    BOTTOM,
    LEFT,
    RIGHT
};

class LineSegment2D {

public:

    glm::vec2 p0;
    glm::vec2 p1;
    Direction layout;
    LineSegment2D() {
    }
    LineSegment2D(float x0, float y0, float x1, float y1) {
        p0 = glm::vec2(x0, y0);
        p1 = glm::vec2(x1, y1);
    }
    
};

#endif