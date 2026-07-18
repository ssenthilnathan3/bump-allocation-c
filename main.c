#include "include/block.h"
#include <stddef.h>

int main(void) {
    char *c = bump_alloc(sizeof(char), _Alignof(char));
    int *i = bump_alloc(sizeof(int), _Alignof(int));
    double *d = bump_alloc(sizeof(double), _Alignof(double));

    bump_free(c);
    bump_free(i);
    bump_free(d);
    return 0;
}
