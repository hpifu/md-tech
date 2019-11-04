#include <stdio.h>
#include <stdlib.h>

struct A {};

int main(int argc, const char* argv[]) {
    printf("sizeof(A): %lu\n", sizeof(struct A));
    // sizeof(A): 0

    struct A a1;
    struct A a2;
    printf("address(a1): %p\n", &a1);
    printf("address(a2): %p\n", &a2);
    printf("&a1 == &a2: %d\n", &a1 == &a2);
    // address(a1): 0x7ffee79bb528
    // address(a2): 0x7ffee79bb520
    // &a1 == &a2: 0

    struct A* a3 = (struct A*)malloc(sizeof(struct A));
    struct A* a4 = (struct A*)malloc(sizeof(struct A));
    printf("address(a3): %p\n", &a3);
    printf("address(a4): %p\n", &a4);
    printf("&a3 == &a4: %d\n", &a3 == &a4);
    // address(a3): 0x7ffee79bb518
    // address(a4): 0x7ffee79bb510
    // &a3 == &a4: 0
    free(a3);
    free(a4);
}
