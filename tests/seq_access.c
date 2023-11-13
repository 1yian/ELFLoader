#define SIZE 1000000  // Adjust size to be large enough
static int array[SIZE];

void compute() {
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += array[i];
        // Perform other computations as needed
    }
}

int main() {
    // Initialize array elements
    for (int i = 0; i < SIZE; i++) {
        array[i] = i;
    }
    compute();
    return 0;
}
