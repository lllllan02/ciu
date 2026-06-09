#include "test.h"
#include "test.c"

int main() {
    setbuf(stdout, NULL);
    printf("Running merge sort (linked list) tests...\n");
    run_all_tests();
    printf("All tests passed!\n");
    return 0;
}
