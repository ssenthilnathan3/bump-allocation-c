#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint8_t* ptr;
  size_t size;
} Block;


typedef struct {
  uint8_t *cursor;
  uint8_t *limit;
  Block block;
  struct FreeNode *free_list;
} Arena;

typedef struct FreeNode {
  size_t size;
  size_t padding;
  struct FreeNode *next;
} FreeNode;

typedef struct FreeFooter {
  size_t size;
} FreeFooter;
