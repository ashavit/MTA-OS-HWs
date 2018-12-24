#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>    // execvp(), pipe()
#include "Commons.h"

int main(int argc, const char * argv[]) {

  printf("DEBUG: WRITER: started on proccess %d\n", getpid());

  if (argc != 2) {
		fprintf(stderr, "Wrong number of Arguments: Missing prefix\n\n");
		exit(MISSING_ARGS);
	}

  printf("DEBUG: WRITER: started with prefix %s\n", argv[1]);

  int bytes;
  char c;
  const char* pre = argv[1];
  printf("%s", pre);

  while ((bytes = read(0, &c, sizeof(c))) > 0) {
    printf("%c", c);
    if (c == '\n')
      printf("%s", pre);
  }

  if (bytes == -1) {
      printf("MAIN: Error reading from pipe");
      exit(PIPE_READ_ERROR);
  }

  // No need for anymore pipes
  close(0);
  close(1);
  return 0;
}
