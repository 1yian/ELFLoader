#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    const char *filename = "test_temp";
    const char *testString = "Hello, this is a test for file I/O operations.";

    // Writing to a file
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        return 1;
    }

    fputs(testString, file);
    fclose(file);

    // Reading from the file
    char buffer[100];
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        return 1;
    }

    if (fgets(buffer, 100, file) == NULL) {
        perror("Error reading from file");
        fclose(file);
        return 1;
    }
    fclose(file);

    return 0;
}
