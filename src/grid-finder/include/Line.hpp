#pragma once

#include <Angle.hpp>
#include <Matrix.hpp>
#include <Pixel.hpp>
#include <ostream>

struct Point {
    float x;
    float y;

    constexpr bool operator==(Point rhs) const {
        return this->x == rhs.x && this->y == rhs.y;
    }
    static Point fromPixel(Pixel a) { return Point{(float) a.x, (float) a.y}; }
    static float distsq(Point a, Point b) {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return dx * dx + dy * dy;
    }
};

inline std::ostream &operator<<(std::ostream &os, Point p) {
    return os << '(' << p.x << ", " << p.y << ')';
}

#include <iostream>

using std::cout;
using std::endl;

class Line {
    using ColVec3f = TColVector<float, 3>;

  public:
    Line(ColVec3f homogCoordinates) : homogCoordinates(homogCoordinates) {}
    Line(Pixel p, CosSin slope)
        : homogCoordinates{
              slope.sinf(),
              -slope.cosf(),
              -slope.sinf() * p.x + slope.cosf() * p.y,
          } {}

    bool rightOfPoint(Pixel point) {
        float signedDistance = homogCoordinates[0] * point.x +
                               homogCoordinates[1] * point.y +
                               homogCoordinates[2];
        return signedDistance >= 0;
    }

    bool leftOfPoint(Pixel point) {
        float signedDistance = homogCoordinates[0] * point.x +
                               homogCoordinates[1] * point.y +
                               homogCoordinates[2];
        return signedDistance <= 0;
    }

    static Point intersect(Line a, Line b) {
        ColVec3f crss = cross(a.homogCoordinates, b.homogCoordinates);
        return {crss[0] / crss[2], crss[1] / crss[2]};
    }

  private:
    const ColVec3f homogCoordinates;
};