#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <ostream>

template <size_t>
class Angle;

struct CosSin {
    int cos;
    int sin;

    [[nodiscard]] constexpr CosSin perpendicular(bool plus90deg = true) const {
        return plus90deg ? CosSin{-sin, cos} : CosSin{sin, -cos};
    }
    [[nodiscard]] constexpr CosSin opposite() const {
        return CosSin{-cos, -sin};
    }
};

inline std::ostream &operator<<(std::ostream &os, CosSin angle) {
    return os << std::atan2(angle.sin, angle.cos);
}

template <size_t Resolution>
class Angle {
  private:
    size_t angle_index;

  public:
    Angle() : angle_index(0) {}
    Angle(size_t angleIndex) : angle_index(angleIndex) {}
    Angle(int angleIndex) : angle_index(angleIndex) {}
    Angle(double angle) : Angle{getIndex(angle)} {};
    Angle(long double angle) : Angle{getIndex(angle)} {};

    static constexpr size_t resolution() { return Resolution; }

    [[nodiscard]] constexpr operator CosSin() const {
        return {cosines[angle_index], sines[angle_index]};
    }

    [[nodiscard]] constexpr explicit operator double() const { return rad(); }

    [[nodiscard]] constexpr double rad() const { return getAngle(angle_index); }

    [[nodiscard]] constexpr size_t getIndex() const { return angle_index; }

    [[nodiscard]] constexpr int cos() const { return cosines[angle_index]; }

    [[nodiscard]] constexpr int sin() const { return sines[angle_index]; }

    [[nodiscard]] constexpr double cosd() const {
        return (double) cos() / getScalingFactor();
    }

    [[nodiscard]] constexpr double sind() const {
        return (double) sin() / getScalingFactor();
    }

    [[nodiscard]] constexpr Angle operator+(Angle rhs) const {
        return normalize(this->angle_index + rhs.angle_index);
    }

    [[nodiscard]] constexpr Angle operator/(size_t divisor) const {
        return angle_index / divisor;
    }

    constexpr bool operator==(Angle rhs) const {
        return this->angle_index == rhs.angle_index;
    }

    [[nodiscard]] static constexpr Angle normalize(size_t angle_index) {
        if (angle_index >= Resolution)
            angle_index -= Resolution;
        return angle_index;
    }

    constexpr static Angle average(Angle first_angle, Angle last_angle) {
        auto first =  first_angle.angle_index;
        auto last = last_angle.angle_index;
        if (first > last)
            last += resolution();
        size_t angle = (first + last) / 2;
        return normalize(angle);
    }

    static constexpr int cos(size_t angleIndex) {
        return std::round(std::cos(step() * angleIndex) * getScalingFactor());
    }

    static constexpr int sin(size_t angleIndex) {
        return std::round(std::sin(step() * angleIndex) * getScalingFactor());
    }

    static constexpr size_t getIndex(double angle) {
        return round(angle / step());
    }

    static constexpr double getAngle(size_t angleIndex) {
        return angleIndex * step();
    }

    static constexpr int getScalingFactor() {
        return std::numeric_limits<int>::max() / 2;
    }

    static constexpr double step() { return 2 * M_PI / Resolution; }

    static constexpr std::array<int, Resolution> calculateCosLut() {
        std::array<int, Resolution> cosines = {};
        for (size_t i = 0; i < Resolution; ++i)
            cosines[i] = cos(i);
        return cosines;
    }

    static constexpr std::array<int, Resolution> calculateSinLut() {
        std::array<int, Resolution> sines = {};
        for (size_t i = 0; i < Resolution; ++i)
            sines[i] = sin(i);
        return sines;
    }

    static constexpr std::array<int, Resolution> cosines = calculateCosLut();
    static constexpr std::array<int, Resolution> sines   = calculateSinLut();
};

using angle_t = Angle<360>;

template <size_t Resolution>
std::ostream &operator<<(std::ostream &os, Angle<Resolution> angle) {
    return os << (double) angle;
}