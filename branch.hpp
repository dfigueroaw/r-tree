#ifndef BRANCH_HPP
#define BRANCH_HPP

#include <memory>
#include <variant>
#include "rect.hpp"

template<typename Coord, std::size_t N, typename T, std::size_t MAX>
class Node;

template<typename Coord, std::size_t N, typename T, std::size_t MAX>
class Branch {
    using NodeType = Node<Coord, N, T, MAX>;
    using NodePtr = std::unique_ptr<NodeType>;
    using VariantType = std::variant<T, NodePtr>;

    Rect<Coord, N> rect{};
    VariantType data{};

public:
    Branch() = default;

    Branch(Rect<Coord, N> rect, T value)
        : rect(std::move(rect)),
          data(std::move(value)) {}

    Branch(Rect<Coord, N> rect, NodePtr child)
        : rect(std::move(rect)),
          data(std::move(child)) {}

    [[nodiscard]] bool is_leaf() const {
        return std::holds_alternative<T>(data);
    }

    [[nodiscard]] bool is_internal() const {
        return std::holds_alternative<NodePtr>(data);
    }

    [[nodiscard]] T& value() {
        return std::get<T>(data);
    }

    [[nodiscard]] const T& value() const {
        return std::get<T>(data);
    }

    [[nodiscard]] NodePtr& child() {
        return std::get<NodePtr>(data);
    }

    [[nodiscard]] const NodePtr& child() const {
        return std::get<NodePtr>(data);
    }

    [[nodiscard]] Rect<Coord, N>& get_rect() {
        return rect;
    }

    [[nodiscard]] const Rect<Coord, N>& get_rect() const {
        return rect;
    }

    void set_rect(Rect<Coord, N> new_rect) {
        rect = std::move(new_rect);
    }
};

#endif
