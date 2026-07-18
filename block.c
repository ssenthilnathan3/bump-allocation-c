#include "include/block.h"
#include "include/block_types.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

BumpBlock arena = { .cursor = NULL, .limit = NULL, .block = NULL };
Block global_block = { .ptr = NULL, .size = 0 };
static FreeNode* free_list = NULL;

int create_arena(void) {
  uint8_t* ptr_val = malloc(BLOCK_SIZE);

  global_block.ptr = ptr_val;
  global_block.size = BLOCK_SIZE;

  arena.block = &global_block;

  arena.cursor = arena.block->ptr;
  arena.limit = arena.block->ptr + BLOCK_SIZE;

  return 1;
}

void reset_arena(void) {
  if (global_block.ptr != NULL) {
      free(global_block.ptr);
  }

  global_block.ptr = NULL;
  global_block.size = 0;

  arena.cursor = NULL;
  arena.limit = NULL;
  arena.block = NULL;

  free_list = NULL;
}

int is_free(FreeNode *h) {
  return h->size & FREE_FLAG;
}

size_t block_size(FreeNode *h) {
  return h->size & SIZE_MASK;
}

FreeFooter* footer_of(FreeNode *h) {
  return (FreeFooter *)((uint8_t *)h + sizeof(FreeNode) + block_size(h) - sizeof(FreeFooter));
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
  uint8_t *candidate = (uint8_t *)block + sizeof(FreeNode) + sizeof(void *);

  size_t padding = (alignment - ((uintptr_t)candidate & (alignment - 1))) & (alignment - 1);

  if (sizeof(void *) + padding + size + sizeof(FreeFooter) <= block_size(block))
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
    header->size &= SIZE_MASK;

    uint8_t *candidate = (uint8_t *)header + sizeof(FreeNode) + sizeof(void *);
    size_t padding = (alignment - ((uintptr_t)candidate & (alignment - 1))) & (alignment - 1);

    size_t needed = sizeof(void *) + padding + size + sizeof(FreeFooter);

    uint8_t *remainder_raw = (uint8_t *)header + sizeof(FreeNode) + needed;
    uintptr_t align = _Alignof(FreeNode);
    uintptr_t misalign = (uintptr_t)remainder_raw & (align - 1);
    size_t fixup = misalign ? (align - misalign) : 0;
    uint8_t *remainder_start = remainder_raw + fixup;

    size_t used_so_far = sizeof(FreeNode) + needed + fixup;

    if (header->size >= used_so_far &&
        header->size - used_so_far >= sizeof(FreeNode) + MIN_SPLIT_SIZE) {
      size_t remainder_size = header->size - used_so_far;

      FreeNode *remainder = (FreeNode *)remainder_start;
      remainder->size = remainder_size;
      remainder->size |= FREE_FLAG;
      remainder->padding = 0;
      write_footer(remainder);
      push_free(remainder);

      header->size = needed + fixup;
    }

    write_footer(header);

    uint8_t *user_ptr = candidate + padding;
    header->padding = padding;
    *(FreeNode **)(user_ptr - sizeof(void *)) = header;

    header->next = NULL;
    return user_ptr;
  }

  header = (FreeNode *)arena.cursor;

  uint8_t *candidate = (uint8_t *)header + sizeof(FreeNode) + sizeof(void *);

  size_t padding = (alignment - ((uintptr_t)candidate & (alignment - 1))) & (alignment - 1);

  uint8_t *user_ptr = candidate + padding;

  uint8_t *end = user_ptr + size + sizeof(FreeFooter);

  uintptr_t align = _Alignof(FreeNode);
  uintptr_t misalign = (uintptr_t)end & (align - 1);
  size_t fixup = misalign ? (align - misalign) : 0;
  end += fixup;

  if (end > arena.limit) {
    return NULL;
  }

  arena.cursor = end;

  header->size = sizeof(void *) + padding + size + sizeof(FreeFooter) + fixup;
  header->padding = padding;
  header->next = NULL;

  write_footer(header);

  *(FreeNode **)(user_ptr - sizeof(void *)) = header;
  return user_ptr;
}

void unlink_from_free_list(FreeNode *node) {
    FreeNode *head = free_list;
    FreeNode *prev = NULL;

    while (head != NULL) {
        if (head == node) {
            if (prev == NULL) {
                free_list = head->next;
            } else {
                prev->next = head->next;
            }
            head->next = NULL;
            return;
        }
        prev = head;
        head = head->next;
    }
}

void write_footer(FreeNode *header) {
    FreeFooter *f = footer_of(header);
    f->size = block_size(header);
}

void bump_free(void *ptr) {
  if (ptr == NULL)
    return;

  FreeNode *header =
    *(FreeNode **)((uint8_t *)ptr - sizeof(void *));
    FreeNode *right = (FreeNode *)((uint8_t *)header + sizeof(FreeNode) + block_size(header));
    if ((uint8_t *)right < arena.cursor && right < (FreeNode *)arena.limit && is_free(right)) {
        unlink_from_free_list(right);
        header->size = block_size(header) + sizeof(FreeNode) + block_size(right);
    }

    if ((uint8_t *)header > arena.block->ptr) {
        FreeFooter *left_footer = (FreeFooter *)((uint8_t *)header - sizeof(FreeFooter));
        FreeNode *left = (FreeNode *)((uint8_t *)header - sizeof(FreeNode) - left_footer->size);
        if ((uint8_t *)left >= arena.block->ptr && (uint8_t *)left < (uint8_t *)header && is_free(left)) {
            unlink_from_free_list(left);
            left->size = block_size(left) + sizeof(FreeNode) + block_size(header);
            header = left;
        }
    }

  header->size |= FREE_FLAG;
  write_footer(header);
  push_free(header);
}

void push_free(FreeNode *node) {
  node->next = free_list;
  free_list = node;
}
