#include "include/block.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void reset(void)
{
    reset_arena();
}


static void test_basic_allocation(void)
{
    printf("Test 1: Basic allocation...\n");

    reset();

    void *a = bump_alloc(32, 8);

    assert(a != NULL);
    assert(((uintptr_t)a % 8) == 0);

    printf("A = %p\n", a);
    printf("PASS\n\n");
}


static void test_multiple_allocations(void)
{
    printf("Test 2: Multiple allocations...\n");

    reset();

    void *a = bump_alloc(32, 8);
    void *b = bump_alloc(64, 8);
    void *c = bump_alloc(16, 16);

    assert(a);
    assert(b);
    assert(c);

    assert(a != b);
    assert(b != c);

    assert(((uintptr_t)b % 8) == 0);
    assert(((uintptr_t)c % 16) == 0);

    printf("A = %p\n", a);
    printf("B = %p\n", b);
    printf("C = %p\n", c);

    printf("PASS\n\n");
}


static void test_reuse(void)
{
    printf("Test 3: Reuse freed block...\n");

    reset();

    void *a = bump_alloc(64, 8);

    bump_free(a);

    void *b = bump_alloc(32, 8);

    assert(b == a);

    printf("Old = %p\n", a);
    printf("New = %p\n", b);

    printf("PASS\n\n");
}


static void test_lifo(void)
{
    printf("Test 4: LIFO free-list...\n");

    reset();

    void *a = bump_alloc(32, 8);
    bump_alloc(8, 8);
    void *b = bump_alloc(32, 8);
    bump_alloc(8, 8);
    void *c = bump_alloc(32, 8);

    bump_free(c);
    bump_free(b);
    bump_free(a);

    void *d = bump_alloc(16, 8);
    void *e = bump_alloc(16, 8);
    void *f = bump_alloc(16, 8);

    assert(d == a);
    assert(e == b);
    assert(f == c);

    printf("PASS\n\n");
}


static void test_back_pointer(void)
{
    printf("Test 5: Back pointer...\n");

    reset();

    void *p = bump_alloc(48, 32);

    FreeNode *header =
        *(FreeNode **)((uint8_t *)p - sizeof(void *));

    assert(header != NULL);

    uint8_t *candidate =
        (uint8_t *)header +
        sizeof(FreeNode) +
        sizeof(void *);
    printf("header          = %p\n", (void *)header);
    printf("candidate       = %p\n", (void *)candidate);
    printf("stored padding  = %zu\n", header->padding);
    printf("expected user   = %p\n", (void *)(candidate + header->padding));
    printf("actual user     = %p\n", p);
    assert(candidate + header->padding == (uint8_t *)p);

    printf("Header = %p\n", (void *)header);
    printf("User   = %p\n", p);

    printf("PASS\n\n");
}


static void test_alignment(void)
{
    printf("Test 6: Alignment...\n");

    reset();

    void *a = bump_alloc(8, 8);
    void *b = bump_alloc(8, 16);
    void *c = bump_alloc(8, 32);
    void *d = bump_alloc(8, 64);

    assert(((uintptr_t)a % 8) == 0);
    assert(((uintptr_t)b % 16) == 0);
    assert(((uintptr_t)c % 32) == 0);
    assert(((uintptr_t)d % 64) == 0);

    printf("PASS\n\n");
}


static void test_split(void)
{
    printf("Test 7: Split block...\n");

    reset();

    void *big = bump_alloc(128, 8);
    assert(big);

    bump_free(big);

    void *small = bump_alloc(32, 8);

    assert(small == big);

    void *rest = bump_alloc(32, 8);

    assert(rest != NULL);
    assert(rest != small);

    printf("Small     = %p\n", small);
    printf("Remainder = %p\n", rest);

    printf("PASS\n\n");
}


static void test_split_reuse(void)
{
    printf("Test 8: Split remainder reuse...\n");

    reset();

    void *big = bump_alloc(128, 8);

    bump_free(big);

    void *second = bump_alloc(32, 8);

    bump_free(second);

    void *third = bump_alloc(16, 8);

    assert(third == second);

    printf("PASS\n\n");
}

static void test_exhaust(void)
{
    printf("Test 9: Exhaust arena...\n");

    reset();

    size_t count = 0;

    while (bump_alloc(64, 8))
        count++;

    assert(bump_alloc(64, 8) == NULL);

    printf("Allocated %zu blocks.\n", count);

    printf("PASS\n\n");
}

static void test_allocate_after_free(void)
{
    printf("Test 10: Allocate after free...\n");

    reset();

    void *a = bump_alloc(64, 8);

    bump_free(a);

    void *b = bump_alloc(16, 8);

    assert(b == a);

    printf("PASS\n\n");
}


static void test_stress(void)
{
    printf("Test 11: Stress...\n");

    reset();

    void *ptrs[128];

    for (int i = 0; i < 128; i++)
        ptrs[i] = bump_alloc(16, 8);

    for (int i = 0; i < 128; i += 2)
        if (ptrs[i])
            bump_free(ptrs[i]);

    for (int i = 0; i < 64; i++)
        assert(bump_alloc(16, 8) != NULL);

    printf("PASS\n\n");
}


int main(void)
{
    printf("========== Bump Allocator Tests ==========\n\n");

    test_basic_allocation();
    test_multiple_allocations();
    test_reuse();
    test_lifo();
    test_back_pointer();
    test_alignment();
    test_split();
    test_split_reuse();
    test_exhaust();
    test_allocate_after_free();
    test_stress();

    printf("=========================================\n");
    printf("All tests passed!\n");

    return 0;
}
