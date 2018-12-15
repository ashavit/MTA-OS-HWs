#include <stdlib.h>
#include <unistd.h>    // execvp(), pipe()
#include <string.h>
#include "Workers.h"

#define N_SIZE 100

void do_reader(int pip[2]) {
    int bytes;
    long long input = 0;
    long long inputEnd = -1;

    close(pip[0]); // Need only to write to pipe

    while (TRUE) {
        scanf("%llx\n", &input);
    	  // printf("reader got string: %llx\n", input);

        if (input == inputEnd) {
            // printf("reader got end input: %llx\n", input);
            break;
        }

        bytes = write(pip[1], &input, sizeof(input));
        // printf("reader sent to main input: %llx (len=%d)\n", input, bytes);
        if (bytes == -1) {
            printf("READER: Error writing to pipe");
            exit(PIPE_WRITE_ERROR);
        }
    }

    close(pip[1]);
    // printf("reader exiting\n");
    exit(EXIT_SUCCESS) ;
}

void do_writer(int pip[2]) {
    int bytes = 0;
    size_t inputLength = 0;
    char input[N_SIZE];

    close(pip[1]); // Need only to read from pipe

    while (TRUE) {
        bytes = read(pip[0], &inputLength, sizeof(inputLength));
        if (bytes <= 0)
            break;
        // printf("WRITER: need to read length of %zu\n", inputLength);

        bytes = read(pip[0], &input, inputLength);
        if (bytes != inputLength)
            break;
        // printf("WRITER: got %d bytes:\n%s", bytes, input);
        printf("%s", input);
    }

    if (bytes == -1) {
        printf("WRITER: Error reading from pipe");
        exit(PIPE_WRITE_ERROR);
    }

    close(pip[0]);
    // printf("writer exiting\n");
    exit(EXIT_SUCCESS) ;
}
