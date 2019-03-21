#pragma once

#include <Angle.hpp>
#include <Matrix.hpp>
#include <Pixel.hpp>
#include <ostream>

struct Point {
    double x;
    double y;

    constexpr bool operator==(Point rhs) const {
        return this->x == rhs.x && this->y == rhs.y;
    }
};

inline std::ostream &operator<<(std::ostream &os, Point p) {
    return os << '(' << p.x << ", " << p.y << ')';
}

class Line {
  private:
    const ColVector<3> homogCoordinates;

  public:
    Line(Pixel p, angle_t slope)
        : homogCoordinates{
              slope.sind(),
              -slope.cosd(),
              -slope.sind() * p.x + slope.cosd() * p.y,
          } {}

    Line(ColVector<3> homogCoordinates) : homogCoordinates(homogCoordinates) {}

    static Point intersect(Line a, Line b) {
        ColVector<3> crss = cross(a.homogCoordinates, b.homogCoordinates);
        return {crss[0] / crss[2], crss[1] / crss[2]};
    }
};