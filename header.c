#include "include/header.h"

FreeNode* user_to_header(uint8_t* ptr) {
  return (FreeNode *)((uintptr_t)ptr - sizeof(FreeNode));
}


uint8_t *header_to_user(FreeNode *header) {
  return (uint8_t *)header + sizeof(FreeNode);
}
