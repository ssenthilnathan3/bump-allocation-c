#include "block.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

BumpBlock arena = { .cursor = NULL, .limit = NULL, .block = NULL };
Block global_block = { .ptr = NULL, .size = 0 };
static FreeNode* free_list = NULL;

FreeNode* user_to_header(uint8_t* ptr) {
  return (FreeNode *)((uintptr_t)ptr - sizeof(FreeNode));
}


uint8_t *header_to_user(FreeNode *header) {
  return (uint8_t *)header + sizeof(FreeNode);
}

int create_arena(void) {
  uint8_t* ptr_val = malloc(BLOCK_SIZE);

  global_block.ptr = ptr_val;
  global_block.size = BLOCK_SIZE;

  arena.block = &global_block;

  arena.cursor = arena.block->ptr;
  arena.limit = arena.block->ptr + BLOCK_SIZE;

  return 1;
}

int check_valid_arena_exists(void) {
  if (arena.cursor == NULL || arena.limit == NULL || arena.block == NULL) {
    return create_arena();
  }

  if (arena.cursor == arena.limit) {
    return -1;
  }
  return 1;
}

FreeNode *check_if_fits(FreeNode *block, size_t alignment, size_t size) {
  uint8_t *candidate = (uint8_t *)block + sizeof(FreeNode);

  candidate += sizeof(void *);

  size_t padding = (alignment - ((uintptr_t)candidate & (alignment - 1))) & (alignment - 1);

  if (sizeof(void *) + padding + size <= block->size)
      return block;

  return NULL;
}

FreeNode* search_free(FreeNode** free_list, size_t size_val, size_t alignment) {
  FreeNode *head = *free_list;
  FreeNode* prev = NULL;

  while (head != NULL) {
    if (check_if_fits(head, alignment, size_val) != NULL) {
      if (prev == NULL) {
        *free_list = head->next;
      } else {
        prev->next = head->next;
      }

      head->next = NULL;
      return head;
    }

    prev = head;
    head = head->next;
  }

  return NULL;
}

void *bump_alloc(size_t size, size_t alignment) {
  if (check_valid_arena_exists() == -1) {
    printf("Error creating arena\n");
    return NULL;
  }

  if (size == 0) {
    printf("Not a valid size to allocate\n");
    return NULL;
  }

  FreeNode *header = search_free(&free_list, size, alignment);

  if (header != NULL) {
    uint8_t *candidate = (uint8_t *)header + sizeof(FreeNode) + sizeof(void *);

    size_t padding = (alignment - ((uintptr_t)candidate & (alignment - 1))) & (alignment - 1);

    uint8_t *user_ptr = candidate + padding;

    *(FreeNode **)(user_ptr - sizeof(void *)) = header;

    header->next = NULL;

    return user_ptr;
  }

  header = (FreeNode *)arena.cursor;

  uint8_t *candidate = (uint8_t *)header + sizeof(FreeNode) + sizeof(void *);

  size_t padding = (alignment - ((uintptr_t)candidate & (alignment - 1))) & (alignment - 1);

  uint8_t *user_ptr = candidate + header->padding;

  uint8_t *end = user_ptr + size;

  if (end > arena.limit) {
    return NULL;
  }

  arena.cursor = end;

  header->size = sizeof(void *) + padding + size;
  header->padding = padding;
  header->next = NULL;

  *(FreeNode **)(user_ptr - sizeof(void *)) = header;

  return user_ptr;
}

void bump_free(void *ptr) {
  if (ptr == NULL)
    return;

  FreeNode *header =
    *(FreeNode **)((uint8_t *)ptr - sizeof(void *));

  push_free(header);
}

void push_free(FreeNode *node) {
  node->next = free_list;
  free_list = node;
}
