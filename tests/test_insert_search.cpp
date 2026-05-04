#include "test_utils.hpp"

void test_basic_insert_search() {
    RTree<double, 2, int, 4> tree;

    tree.insert(
        {
            {0, 0},
            {1, 1}
    },
        1);
    tree.insert(
        {
            {2, 2},
            {3, 3}
    },
        2);

    auto r = tree.search({
        {0, 0},
        {1, 1}
    });
    assert(r.size() == 1 && r[0] == 1);
}

void test_overlap_and_containment() {
    RTree<double, 2, int, 4> tree;

    tree.insert(
        {
            {0, 0},
            {2, 2}
    },
        1);
    tree.insert(
        {
            {1, 1},
            {3, 3}
    },
        2);

    auto r = tree.search({
        {  1,   1},
        {1.5, 1.5}
    });
    assert_contains_all(r, {1, 2});
}

void test_split_and_large_insert() {
    auto tree = make_linear_tree(10);
    auto r = tree.search({
        { 0,  0},
        {10, 10}
    });
    assert(r.size() == 10);
}

void test_edge_touching() {
    RTree<double, 2, int, 4> tree;

    tree.insert(
        {
            {0, 0},
            {1, 1}
    },
        1);
    auto r = tree.search({
        {1, 1},
        {2, 2}
    });

    assert(r.size() == 1);
}

void test_identical_and_nested() {
    RTree<double, 2, int, 4> tree;

    auto r = Rect<double, 2>({0, 0}, {1, 1});
    tree.insert(r, 1);
    tree.insert(r, 2);

    tree.insert(
        {
            { 0,  0},
            {10, 10}
    },
        3);

    auto res = tree.search({
        {0.5, 0.5},
        {0.6, 0.6}
    });
    assert_contains_all(res, {1, 2, 3});
}

void test_empty_query() {
    auto tree = make_linear_tree(5);
    auto r = tree.search({
        {100, 100},
        {200, 200}
    });
    assert(r.empty());
}

void test_insert_search() {
    test_basic_insert_search();
    test_overlap_and_containment();
    test_split_and_large_insert();
    test_edge_touching();
    test_identical_and_nested();
    test_empty_query();
}
