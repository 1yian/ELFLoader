#include <stdio.h>

#define SIZE 1000//1000000  // Total size of the array
#define STEP 5//50000    // Step size for sparse access
#define SEQ_LEN 10   // Length of each sequential access

static int array[SIZE];

int main() {
    // Sparse sequential access
    for (int i = 0; i < SIZE; i += STEP) {
        // Sequential access within a small range
        for (int j = i; j < i + SEQ_LEN && j < SIZE; j++) {
            array[j] = j;  // Example operation
        }
    }

    // Optional: Print some values to verify (can be commented out)
    // for (int i = 0; i < SIZE; i += STEP) {
    //     printf("array[%d] = %d\n", i, array[i]);
    // }
    printf("Done\n");
    return 0;

}
