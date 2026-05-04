#include "test_utils.hpp"

void test_basic_remove() {
    auto tree = make_linear_tree(3);

    assert(tree.remove(1));
    auto r = tree.search({
        {1, 1},
        {1, 1}
    });
    assert(r.empty());
}

void test_remove_nonexistent() {
    auto tree = make_linear_tree(3);
    assert(!tree.remove(42));
}

void test_remove_with_rect() {
    RTree<double, 2, int, 4> tree;

    auto r1 = Rect<double, 2>({0, 0}, {2, 2});
    auto r2 = Rect<double, 2>({1, 1}, {3, 3});

    tree.insert(r1, 1);
    tree.insert(r2, 2);

    tree.remove(r1, 1);

    auto r = tree.search(r1);
    assert(r.size() == 1 && r[0] == 2);
}

void test_underflow_and_reinsert() {
    auto tree = make_linear_tree(8);

    tree.remove(0);
    tree.remove(1);
    tree.remove(2);

    auto r = tree.search({
        { 0,  0},
        {10, 10}
    });
    assert(r.size() == 5);
}

void test_root_collapse_and_full_delete() {
    auto tree = make_linear_tree(6);

    for (int i = 0; i < 5; ++i)
        tree.remove(i);

    auto r = tree.search({
        { 0,  0},
        {10, 10}
    });
    assert(r.size() == 1);

    tree.remove(5);
    assert(tree.search({
                           { 0,  0},
                           {10, 10}
    })
               .empty());
}

void test_duplicates_and_partial_remove() {
    RTree<double, 2, int, 4> tree;

    auto r = Rect<double, 2>({0, 0}, {1, 1});
    tree.insert(r, 1);
    tree.insert(r, 1);

    tree.remove(1);

    auto res = tree.search(r);
    assert(res.size() == 1);
}

void test_heavy_overlap_delete() {
    RTree<double, 2, int, 4> tree;

    for (int i = 0; i < 20; ++i)
        tree.insert(
            {
                { 0,  0},
                {10, 10}
        },
            i);

    for (int i = 0; i < 10; ++i)
        tree.remove(i);

    auto r = tree.search({
        {5, 5},
        {6, 6}
    });
    assert(r.size() == 10);
}

void test_interleaved_operations() {
    RTree<double, 2, int, 4> tree;

    for (int i = 0; i < 20; ++i) {
        tree.insert(
            {
                {static_cast<double>(i), static_cast<double>(i)},
                {static_cast<double>(i), static_cast<double>(i)}
        },
            i);
        if (i % 3 == 0)
            tree.remove(i);
    }

    auto r = tree.search({
        { 0,  0},
        {30, 30}
    });
    assert(r.size() == 13);
}

void test_delete() {
    test_basic_remove();
    test_remove_nonexistent();
    test_remove_with_rect();
    test_underflow_and_reinsert();
    test_root_collapse_and_full_delete();
    test_duplicates_and_partial_remove();
    test_heavy_overlap_delete();
    test_interleaved_operations();
}
