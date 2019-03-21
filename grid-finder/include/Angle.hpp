#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

template <size_t Resolution>
class Angle {
  private:
    const size_t angle_index;

  public:
    Angle(size_t angleIndex) : angle_index(angleIndex) {}
    Angle(int angleIndex) : angle_index(angleIndex) {}
    Angle(double angle) : Angle{getIndex(angle)} {};

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
        if (first_angle > last_angle)
            last_angle += 2 * M_PI;
        Angle angle = (first_angle + last_angle) / 2;
        return normalize(angle);
    }

    static constexpr int cos(size_t angleIndex) {
        return std::round(std::cos(getStep() * angleIndex) *
                          getScalingFactor());
    }

    static constexpr int sin(size_t angleIndex) {
        return std::round(std::sin(getStep() * angleIndex) *
                          getScalingFactor());
    }

    static constexpr size_t getIndex(double angle) {
        return round(angle / getStep());
    }

    static constexpr double getAngle(size_t angleIndex) {
        return angleIndex * getStep();
    }

    static constexpr int getScalingFactor() {
        return std::numeric_limits<int>::max() / 2;
    }

    static constexpr double getStep() { return 2 * M_PI / Resolution; }

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