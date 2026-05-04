#ifndef RTREE_HPP
#define RTREE_HPP

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <queue>
#include <stdexcept>
#include <vector>
#include "node.hpp"
#include "rect.hpp"

template<typename Coord, std::size_t N, typename T, std::size_t MAX>
class RTree {
public:
    using NodeType = Node<Coord, N, T, MAX>;
    using NodePtr = std::unique_ptr<NodeType>;
    using BranchType = Branch<Coord, N, T, MAX>;

    enum class Group : std::int8_t {
        Unassigned = -1,
        A = 0,
        B = 1
    };

    struct Partition {
        std::array<BranchType, MAX + 1> buffer;
        std::array<Group, MAX + 1> group;
        std::size_t total = 0;
        std::size_t min_fill = MAX / 2;
        std::array<Rect<Coord, N>, 2> cover;
        std::array<Coord, 2> area{};
        std::array<std::size_t, 2> count{};
    };

    struct KNNEntry {
        const BranchType* branch;
        Coord distance;
    };

private:
    NodePtr root;

    Rect<Coord, N> node_cover(const NodeType& node) {
        return node.compute_mbr();
    }

    bool add_branch(NodeType* node, BranchType&& branch, NodePtr& new_node) {
        if (!node->is_full()) {
            node->add_branch(std::move(branch));
            return false;
        }
        split_node(node, std::move(branch), new_node);
        return true;
    }

    void split_node(NodeType* node, BranchType new_branch, NodePtr& out_new_node) {
        Partition partition;

        for (std::size_t i = 0; i < node->size(); ++i) {
            partition.buffer[i] = std::move(node->at(i));
        }
        partition.buffer[node->size()] = std::move(new_branch);
        partition.total = node->size() + 1;
        partition.group.fill(Group::Unassigned);

        pick_seeds(partition);
        choose_partition(partition);

        out_new_node = std::make_unique<NodeType>();
        out_new_node->set_level(node->get_level());
        node->clear();

        for (std::size_t i = 0; i < partition.total; ++i) {
            if (partition.group[i] == Group::A) {
                node->add_branch(std::move(partition.buffer[i]));
            } else {
                out_new_node->add_branch(std::move(partition.buffer[i]));
            }
        }
    }

    void pick_seeds(Partition& partition) {
        Coord worst = std::numeric_limits<Coord>::lowest();
        std::size_t seed1 = 0;
        std::size_t seed2 = 1;

        for (std::size_t i = 0; i < partition.total - 1; ++i) {
            for (std::size_t j = i + 1; j < partition.total; ++j) {
                const auto combined =
                    partition.buffer[i].get_rect().merge(partition.buffer[j].get_rect());
                const Coord waste = combined.volume() - partition.buffer[i].get_rect().volume() -
                                    partition.buffer[j].get_rect().volume();

                if (waste > worst) {
                    worst = waste;
                    seed1 = i;
                    seed2 = j;
                }
            }
        }

        assign(partition, seed1, Group::A);
        assign(partition, seed2, Group::B);
    }

    void assign(Partition& partition, std::size_t idx, Group group) {
        partition.group[idx] = group;
        auto group_idx = static_cast<std::size_t>(group);
        partition.cover[group_idx] =
            (partition.count[group_idx] == 0)
                ? partition.buffer[idx].get_rect()
                : partition.cover[group_idx].merge(partition.buffer[idx].get_rect());

        partition.area[group_idx] = partition.cover[group_idx].volume();
        partition.count[group_idx]++;
    }

    void choose_partition(Partition& partition) {
        auto assign_remaining = [&](Group group) {
            for (std::size_t i = 0; i < partition.total; ++i) {
                if (partition.group[i] == Group::Unassigned) {
                    assign(partition, i, group);
                }
            }
        };

        auto pick_next = [&]() -> std::pair<std::size_t, Group> {
            Coord best_diff = std::numeric_limits<Coord>::max();
            std::size_t best_idx = 0;
            Group best_group = Group::Unassigned;

            for (std::size_t i = 0; i < partition.total; ++i) {
                if (partition.group[i] != Group::Unassigned) {
                    continue;
                }

                const auto& rect = partition.buffer[i].get_rect();

                auto merged0 = partition.cover[0].merge(rect);
                auto merged1 = partition.cover[1].merge(rect);

                Coord cost0 = merged0.volume() - partition.area[0];
                Coord cost1 = merged1.volume() - partition.area[1];

                Coord diff = std::abs(cost0 - cost1);
                Group preferred = (cost0 < cost1) ? Group::A : Group::B;

                if (diff < best_diff) {
                    best_diff = diff;
                    best_idx = i;
                    best_group = preferred;
                }
            }

            return {best_idx, best_group};
        };

        while (partition.count[0] + partition.count[1] < partition.total) {
            if (partition.count[0] >= partition.total - partition.min_fill) {
                assign_remaining(Group::B);
                break;
            }

            if (partition.count[1] >= partition.total - partition.min_fill) {
                assign_remaining(Group::A);
                break;
            }

            auto [idx, group] = pick_next();
            assign(partition, idx, group);
        }
    }

    bool insert_recursive(NodeType* node,
                          BranchType&& branch,
                          NodePtr& new_node,
                          std::size_t target_level) {
        if (node->get_level() > target_level) {
            std::size_t idx = node->choose_subtree(branch.get_rect());
            auto* child = node->at(idx).child().get();

            NodePtr split_node_ptr;
            Rect original_rect = branch.get_rect();
            bool child_split =
                insert_recursive(child, std::move(branch), split_node_ptr, target_level);

            if (!child_split) {
                node->at(idx).set_rect(node->at(idx).get_rect().merge(original_rect));
                return false;
            }

            node->at(idx).set_rect(node_cover(*child));

            auto new_branch_cover = node_cover(*split_node_ptr);
            BranchType new_branch{new_branch_cover, std::move(split_node_ptr)};

            return add_branch(node, std::move(new_branch), new_node);
        }

        if (node->get_level() == target_level) {
            return add_branch(node, std::move(branch), new_node);
        }

        throw std::runtime_error("Invalid tree level in insert");
    }

    void reinsert(NodePtr node, std::vector<NodePtr>& list) {
        list.push_back(std::move(node));
    }

    bool remove_recursive(const Rect<Coord, N>* rect,
                          const T& value,
                          NodeType* node,
                          std::vector<NodePtr>& reinsert_list) {
        if (!node->is_leaf()) {
            for (std::size_t i = 0; i < node->size(); ++i) {
                auto& branch = node->at(i);

                if ((rect == nullptr) || branch.get_rect().intersects(*rect)) {
                    if (!remove_recursive(rect, value, branch.child().get(), reinsert_list)) {
                        auto& child_ptr = branch.child();

                        if (child_ptr->size() >= MAX / 2) {
                            branch.set_rect(node_cover(*child_ptr));
                        } else {
                            reinsert(std::move(child_ptr), reinsert_list);
                            node->remove_at(i);
                        }
                        return false;
                    }
                }
            }
            return true;
        }
        for (std::size_t i = 0; i < node->size(); ++i) {
            auto& branch = node->at(i);

            if (branch.is_leaf() && branch.value() == value) {
                node->remove_at(i);
                return false;
            }
        }
        return true;
    }

    bool remove(const Rect<Coord, N>* rect, const T& value) {
        if (!root) {
            return false;
        }

        std::vector<NodePtr> reinsert_list;

        bool not_found = remove_recursive(rect, value, root.get(), reinsert_list);

        if (!not_found) {
            for (auto& node : reinsert_list) {
                for (std::size_t i = 0; i < node->size(); ++i) {
                    NodePtr new_node;
                    insert_recursive(root.get(), std::move(node->at(i)), new_node,
                                     node->get_level());
                }
            }

            if (root->size() == 1 && !root->is_leaf()) {
                root = std::move(root->at(0).child());
            }

            return true;
        }

        return false;
    }

    Coord rect_distance_sq(const Rect<Coord, N>& a, const Rect<Coord, N>& b) const {
        return a.min_distance_sq(b);
    }

    std::vector<std::pair<T, Coord>> knn(const Rect<Coord, N>& query, std::size_t k) const {
        std::vector<std::pair<T, Coord>> result;

        if (!root || k == 0) {
            return result;
        }

        auto cmp = [](const KNNEntry& a, const KNNEntry& b) { return a.distance > b.distance; };
        std::priority_queue<KNNEntry, std::vector<KNNEntry>, decltype(cmp)> pq(cmp);

        for (std::size_t i = 0; i < root->size(); ++i) {
            const auto& branch = root->at(i);
            Coord dist = rect_distance_sq(query, branch.get_rect());
            pq.push(KNNEntry{&branch, dist});
        }

        while (!pq.empty() && result.size() < k) {
            auto current = pq.top();
            pq.pop();

            const BranchType* branch = current.branch;

            if (branch->is_internal()) {
                const auto* node = branch->child().get();

                for (std::size_t i = 0; i < node->size(); ++i) {
                    const auto& child_branch = node->at(i);
                    Coord dist = rect_distance_sq(query, child_branch.get_rect());

                    pq.push(KNNEntry{&child_branch, dist});
                }
            } else {
                result.emplace_back(branch->value(), current.distance);
            }
        }

        return result;
    }

public:
    RTree() = default;

    std::vector<T> search(const Rect<Coord, N>& rect) const {
        std::vector<T> results;
        if (root) {
            root->search(rect, results);
        }
        return results;
    }

    void insert(const Rect<Coord, N>& rect, T value) {
        if (!root) {
            root = std::make_unique<NodeType>();
            root->set_level(0);
        }

        BranchType branch{rect, std::move(value)};
        NodePtr new_node;

        bool root_split = insert_recursive(root.get(), std::move(branch), new_node, 0);

        if (root_split) {
            auto new_root = std::make_unique<NodeType>();
            new_root->set_level(root->get_level() + 1);

            auto old_cover = node_cover(*root.get());
            new_root->add_branch(BranchType{old_cover, std::move(root)});

            auto new_cover = node_cover(*new_node.get());
            new_root->add_branch(BranchType{new_cover, std::move(new_node)});

            root = std::move(new_root);
        }
    }

    bool remove(const T& value) {
        return remove(nullptr, value);
    }

    bool remove(const Rect<Coord, N>& rect, const T& value) {
        return remove(&rect, value);
    }

    std::vector<T> knn_search(const Rect<Coord, N>& query, std::size_t k) const {
        auto pairs = knn(query, k);

        std::vector<T> result;
        result.reserve(pairs.size());

        for (auto& [value, _] : pairs) {
            result.push_back(value);
        }

        return result;
    }
};

#endif
