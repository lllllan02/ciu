#include "test.h"
#include "test.c"

int main() {
    setbuf(stdout, NULL);
    printf("Running balanced BST tests...\n");
    run_all_tests();
    printf("All tests passed!\n");
    return 0;
}
