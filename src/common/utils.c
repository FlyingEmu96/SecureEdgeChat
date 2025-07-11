#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "safe_malloc: failed to allocate %zu bytes\n", size);
        exit(EXIT_FAILURE);
    }
    memset(ptr, 0, size);
    return ptr;
}

