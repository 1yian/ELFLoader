#define SIZE 1000000  // Large size
static int array[SIZE];

int main() {
    // Accessing only a few elements at large intervals
    for (int i = 0; i < SIZE; i += 50000) {  // Adjust the step size as needed
        array[i] = i;
    }
    return 0;
}