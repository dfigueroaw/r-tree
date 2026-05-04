#include <algorithm>
#include <cmath>
#include <cstddef>
#include "test_utils.hpp"

void test_knn_matches_bruteforce() {
    auto tree = make_linear_tree(10);

    double qx = 2.2;
    double qy = 2.2;
    std::size_t k = 5;

    auto r = tree.knn_search(
        {
            {qx, qy},
            {qx, qy}
    },
        k);

    std::vector<std::pair<double, int>> v;
    for (int i = 0; i < 10; ++i) {
        v.emplace_back(std::sqrt(((qx - i) * (qx - i)) + ((qy - i) * (qy - i))), i);
    }

    std::sort(v.begin(), v.end());

    std::vector<int> e;
    for (std::size_t i = 0; i < std::min<std::size_t>(k, v.size()); ++i) {
        e.push_back(v[i].second);
    }

    assert_equal_vectors(r, e);
}

void test_knn_order_from_origin() {
    auto tree = make_linear_tree(6);

    auto r = tree.knn_search(
        {
            {0, 0},
            {0, 0}
    },
        4);
    std::vector<int> e = {0, 1, 2, 3};

    assert_equal_vectors(r, e);
}

void test_knn_query_offset() {
    auto tree = make_linear_tree(10);

    auto r = tree.knn_search(
        {
            {5.1, 5.1},
            {5.1, 5.1}
    },
        3);
    std::vector<int> e = {5, 6, 4};

    assert_equal_vectors(r, e);
}

void test_knn_ties() {
    RTree<double, 2, int, 4> tree;

    tree.insert(
        {
            {1, 0},
            {1, 0}
    },
        1);
    tree.insert(
        {
            {-1, 0},
            {-1, 0}
    },
        2);

    auto r = tree.knn_search(
        {
            {0, 0},
            {0, 0}
    },
        2);

    assert_contains_all(r, {1, 2});
}

void test_knn_rectangles() {
    RTree<double, 2, int, 4> tree;

    tree.insert(
        {
            {0, 0},
            {5, 5}
    },
        1);
    tree.insert(
        {
            {10, 10},
            {15, 15}
    },
        2);

    auto r = tree.knn_search(
        {
            {6, 6},
            {6, 6}
    },
        1);

    assert(r[0] == 1);
}

void test_knn_large_k() {
    auto tree = make_linear_tree(5);

    auto r = tree.knn_search(
        {
            {0, 0},
            {0, 0}
    },
        100);
    std::vector<int> e = {0, 1, 2, 3, 4};

    assert_equal_vectors(r, e);
}

void test_knn_after_remove() {
    auto tree = make_linear_tree(5);
    tree.remove(0);

    auto r = tree.knn_search(
        {
            {0, 0},
            {0, 0}
    },
        3);
    std::vector<int> e = {1, 2, 3};

    assert_equal_vectors(r, e);
}

void test_knn_precision() {
    RTree<double, 2, int, 4> tree;

    tree.insert(
        {
            {0, 0},
            {0, 0}
    },
        1);
    tree.insert(
        {
            {1e-12, 0},
            {1e-12, 0}
    },
        2);

    auto r = tree.knn_search(
        {
            {0, 0},
            {0, 0}
    },
        2);

    assert(r[0] == 1);
    assert(r[1] == 2);
}

void test_knn() {
    test_knn_matches_bruteforce();
    test_knn_order_from_origin();
    test_knn_query_offset();
    test_knn_ties();
    test_knn_rectangles();
    test_knn_large_k();
    test_knn_after_remove();
    test_knn_precision();
}
