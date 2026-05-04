#include <iostream>

void test_insert_search();
void test_delete();
void test_knn();

namespace {
    void run_test(const char* name, void (*fn)()) {
        std::cout << "[RUN] " << name << "\n";
        fn();
        std::cout << "[PASS] " << name << "\n";
    }
}

int main() {
    std::cout << "===== RTree Tests =====\n";

    run_test("Insert/Search", test_insert_search);
    run_test("Delete", test_delete);
    run_test("KNN", test_knn);

    std::cout << "All tests passed successfully\n";
    return 0;
}
