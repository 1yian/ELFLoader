#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define SIZE 64
#define MAGIC 0xCC // The same value used in your interceptor

int main() {
    unsigned char* mem = (unsigned char*)malloc(SIZE);
    if (mem == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // Check if each byte in the allocated memory is set to MAGIC
    for (int i = 0; i < SIZE; i++) {
        assert(mem[i] == MAGIC);
    }

    printf("All allocated memory is correctly initialized to MAGIC value.\n");

    free(mem);
    return 0;
}
