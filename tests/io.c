#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
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

    printf("Read from file: %s\n", buffer);
    fclose(file);

    return 0;
}
