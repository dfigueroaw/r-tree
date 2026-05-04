#ifndef RECT_HPP
#define RECT_HPP

#include <algorithm>
#include <array>

template<typename Coord, std::size_t N>
class Rect {
    std::array<Coord, N> min{};
    std::array<Coord, N> max{};

public:
    Rect() = default;

    Rect(std::array<Coord, N> min, std::array<Coord, N> max)
        : min(std::move(min)),
          max(std::move(max)) {}

    [[nodiscard]] Coord volume() const {
        Coord vol{1};
        for (std::size_t i = 0; i < N; ++i) {
            vol *= (max[i] - min[i]);
        }
        return vol;
    }

    [[nodiscard]] Rect merge(const Rect& other) const {
        Rect result{};
        for (std::size_t i = 0; i < N; ++i) {
            result.min[i] = std::min(min[i], other.min[i]);
            result.max[i] = std::max(max[i], other.max[i]);
        }
        return result;
    }

    [[nodiscard]] bool intersects(const Rect& other) const {
        for (std::size_t i = 0; i < N; ++i) {
            if (max[i] < other.min[i] || other.max[i] < min[i]) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] Coord min_distance_sq(const Rect& other) const {
        Coord dist{};

        for (std::size_t i = 0; i < N; ++i) {
            Coord diff1 = min[i] - other.max[i];
            Coord diff2 = max[i] - other.min[i];

            if (diff1 * diff2 < 0) {
                continue;
            }

            Coord d1 = diff1 * diff1;
            Coord d2 = diff2 * diff2;

            dist += std::min(d1, d2);
        }

        return dist;
    }
};

#endif
