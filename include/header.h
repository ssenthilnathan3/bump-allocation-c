#pragma  once

#include <stdint.h>
#include "block_types.h"

FreeNode* user_to_header(uint8_t* ptr);
uint8_t* header_to_user(FreeNode* header);


