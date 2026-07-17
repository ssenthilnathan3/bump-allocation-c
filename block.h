#include <stddef.h>
#include <stdint.h>

#define BLOCK_SIZE_BITS 15
#define BLOCK_SIZE (1u << BLOCK_SIZE_BITS)

typedef struct {
  uint8_t* ptr;
  size_t size;
} Block;


typedef struct {
  uint8_t *cursor;
  uint8_t *limit;
  Block *block;
} BumpBlock;

typedef struct FreeNode {
  size_t size;
  size_t padding;
  struct FreeNode *next;
} FreeNode;

typedef enum {
  OOM = 0
} AllocError;

void* bump_alloc(size_t size, size_t alignment);
void push_free(FreeNode* node);
void bump_free(void* ptr);

