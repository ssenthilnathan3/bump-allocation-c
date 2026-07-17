#include "block.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    printf("========== Bump Allocator Tests ==========\n\n");

    printf("Test 1: Basic allocation...\n");

    void *a = bump_alloc(32, 8);
    assert(a != NULL);
    assert(((uintptr_t)a % 8) == 0);

    printf("A = %p\n", a);
    printf("PASS\n\n");

    printf("Test 2: Multiple allocations...\n");

    void *b = bump_alloc(64, 8);
    void *c = bump_alloc(16, 16);

    assert(b != NULL);
    assert(c != NULL);

    assert(a != b);
    assert(b != c);

    assert(((uintptr_t)b % 8) == 0);
    assert(((uintptr_t)c % 16) == 0);

    printf("B = %p\n", b);
    printf("C = %p\n", c);
    printf("PASS\n\n");

    printf("Test 3: Reuse freed block...\n");

    bump_free(a);

    void *d = bump_alloc(16, 8);

    printf("Old A = %p\n", a);
    printf("New D = %p\n", d);

    assert(d == a);

    printf("PASS\n\n");

    printf("Test 4: LIFO free-list behaviour...\n");

    bump_free(b);
    bump_free(c);

    void *e = bump_alloc(16, 8);
    void *f = bump_alloc(16, 8);

    printf("Expected first reuse: %p\n", c);
    printf("Actual             : %p\n", e);

    printf("Expected second reuse: %p\n", b);
    printf("Actual              : %p\n", f);

    assert(e == c);
    assert(f == b);

    printf("PASS\n\n");

    printf("Test 5: Exhaust arena...\n");

    size_t count = 0;

    while (bump_alloc(64, 8) != NULL)
        count++;

    printf("Allocated %zu additional blocks before exhaustion.\n", count);

    printf("PASS\n\n");

    printf("Test 6: Allocate after freeing...\n");

    bump_free(e);

    void *g = bump_alloc(8, 8);

    assert(g == e);

    printf("Reused address = %p\n", g);

    printf("PASS\n\n");

    void *x = bump_alloc(16, 8);
    void *y = bump_alloc(16, 8);   // adjacent to x in memory
    bump_free(x);
    bump_free(y);
    void *z = bump_alloc(40, 8);   // bigger than either x or y alone, but fits if merged
    assert(z != NULL);             // will fail — no single free block is big enough

    printf("=========================================\n");
    printf("All tests passed!\n");

    return 0;
}
