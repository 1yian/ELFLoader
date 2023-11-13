#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#define MAGIC 0xCC

void *malloc(size_t size) {
    printf("malloc()\n");
    void *(*real_malloc)(size_t) = dlsym(RTLD_NEXT, "malloc");
    void *p = real_malloc(size);

    printf("malloc(%zu) = %p\n", size, p);

    if (p) {
        memset(p, MAGIC, size);
    }

    return p;
}
