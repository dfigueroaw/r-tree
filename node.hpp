#ifndef NODE_HPP
#define NODE_HPP

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>
#include "branch.hpp"
#include "rect.hpp"

template<typename Coord, std::size_t N, typename T, std::size_t MAX>
class Node {
    using BranchType = Branch<Coord, N, T, MAX>;

    std::array<BranchType, MAX> branches{};
    std::size_t count{0};
    std::size_t level{0};

public:
    [[nodiscard]] bool is_leaf() const {
        return level == 0;
    }

    [[nodiscard]] bool is_full() const {
        return count == MAX;
    }

    [[nodiscard]] std::size_t size() const {
        return count;
    }

    [[nodiscard]] std::size_t get_level() const {
        return level;
    }

    void set_level(std::size_t new_level) {
        level = new_level;
    }

    void clear() {
        count = 0;
    }

    void add_branch(BranchType&& branch) {
        if (is_full()) {
            throw std::runtime_error("Node overflow");
        }
        branches[count++] = std::move(branch);
    }

    const BranchType& at(std::size_t i) const {
        return branches.at(i);
    }

    BranchType& at(std::size_t i) {
        return branches.at(i);
    }

    [[nodiscard]] Rect<Coord, N> compute_mbr() const {
        if (count == 0) {
            throw std::runtime_error("Empty node has no MBR");
        }
        Rect<Coord, N> result = branches[0].get_rect();
        for (std::size_t i = 1; i < count; ++i) {
            result = result.merge(branches[i].get_rect());
        }
        return result;
    }

    [[nodiscard]] std::size_t choose_subtree(const Rect<Coord, N>& rect) const {
        if (is_leaf()) {
            throw std::runtime_error("choose_subtree called on leaf node");
        }

        if (count == 0) {
            throw std::runtime_error("Internal node has no branches");
        }

        std::size_t best_index = 0;
        Coord best_enlargement = std::numeric_limits<Coord>::max();
        Coord best_volume = std::numeric_limits<Coord>::max();

        for (std::size_t i = 0; i < count; ++i) {
            const Rect<Coord, N>& current_rect = branches[i].get_rect();
            const Coord current_volume = current_rect.volume();
            const Coord current_enlargement = current_rect.merge(rect).volume() - current_volume;

            if (current_enlargement < best_enlargement ||
                (current_enlargement == best_enlargement && current_volume < best_volume)) {
                best_index = i;
                best_enlargement = current_enlargement;
                best_volume = current_volume;
            }
        }
        return best_index;
    }

    void search(const Rect<Coord, N>& rect, std::vector<T>& results) const {
        for (std::size_t i = 0; i < count; ++i) {
            const auto& branch = branches[i];
            if (!branch.get_rect().intersects(rect)) {
                continue;
            }
            if (branch.is_leaf()) {
                results.push_back(branch.value());
            } else {
                branch.child()->search(rect, results);
            }
        }
    }

    void remove_at(std::size_t idx) {
        if (idx >= count) {
            throw std::out_of_range("remove_at out of range");
        }
        branches[idx] = std::move(branches[count - 1]);
        --count;
    }
};

#endif
