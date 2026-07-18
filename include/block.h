#pragma once

#include <stddef.h>
#include <stdint.h>
#include "block_types.h"

#define BLOCK_SIZE_BITS 15
#define BLOCK_SIZE (1u << BLOCK_SIZE_BITS)
#define MIN_SPLIT_SIZE sizeof(FreeNode)
#define FREE_FLAG 0x01
#define SIZE_MASK (~(size_t)FREE_FLAG)

int arena_init(Arena *a);
void arena_reset(Arena *a);

void *arena_alloc(Arena *a, size_t size, size_t alignment);
void arena_free(Arena *a, void *ptr);

FreeNode *check_if_fits(FreeNode *block, size_t alignment, size_t size);
FreeNode *search_free(FreeNode **free_list, size_t size_val, size_t alignment);

int is_free(FreeNode *h);
size_t block_size(FreeNode *h);
FreeFooter *footer_of(FreeNode *h);

void push_free(FreeNode **free_list, FreeNode *node);
void write_footer(FreeNode *header);
