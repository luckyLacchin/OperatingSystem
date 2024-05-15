#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <source_file> <destination_file> <mode>\n", argv[0]);
        printf("Mode: 0 for line by line, 1 for char by char\n");
        return 1;
    }

    // Open the source file for reading using file streams
    FILE *sourceFile = fopen(argv[1], "r");
    if (sourceFile == NULL) {
        perror("Error opening source file");
        return 1;
    }

    // Open the destination file for writing using file descriptors
    int destFile = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (destFile < 0) {
        perror("Error opening destination file");
        fclose(sourceFile);
        return 1;
    }

    int mode = atoi(argv[3]);

    if (mode == 0) {
        // Line by line copying
        char buffer[BUFFER_SIZE];

        while (fgets(buffer, BUFFER_SIZE, sourceFile) != NULL) {
            if (write(destFile, buffer, strlen(buffer)) != strlen(buffer)) {
                perror("Error writing to destination file");
                fclose(sourceFile);
                close(destFile);
                return 1;
            }
        }
    } else if (mode == 1) {
        // Character by character copying
        char ch;

        while ((ch = fgetc(sourceFile)) != EOF) {
            if (write(destFile, &ch, 1) != 1) {
                perror("Error writing to destination file");
                fclose(sourceFile);
                close(destFile);
                return 1;
            }
        }
    } else {
        fprintf(stderr,"Invalid mode. Mode: 0 for line by line, 1 for char by char\n");
        fclose(sourceFile);
        close(destFile);
        return 1;
    }

    // Close the files
    fclose(sourceFile);
    close(destFile);

    printf("File copied successfully.\n");
    return 0;
}
