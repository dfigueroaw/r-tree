#ifndef TEST_UTILS_HPP
#define TEST_UTILS_HPP

#include <cassert>
#include <iostream>
#include <vector>
#include "../r_tree.hpp"

template<typename T>
void print_results(const std::vector<T>& results) {
    std::cout << "Results (" << results.size() << "): ";
    for (const auto& v : results) {
        std::cout << v << " ";
    }
    std::cout << "\n";
}

template<typename T>
void assert_contains_all(const std::vector<T>& results, const std::vector<T>& expected) {
    assert(results.size() == expected.size());
    for (const auto& v : expected) {
        assert(std::find(results.begin(), results.end(), v) != results.end());
    }
}

template<typename T>
void assert_equal_vectors(const std::vector<T>& a, const std::vector<T>& b) {
    assert(a.size() == b.size());
    for (size_t i = 0; i < a.size(); ++i) {
        assert(a[i] == b[i]);
    }
}

inline RTree<double, 2, int, 4> make_linear_tree(int n) {
    RTree<double, 2, int, 4> tree;
    for (int i = 0; i < n; ++i) {
        tree.insert(
            {
                {static_cast<double>(i), static_cast<double>(i)},
                {static_cast<double>(i), static_cast<double>(i)}
        },
            i);
    }
    return tree;
}

#endif
