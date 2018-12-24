#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>    // execvp(), pipe()
#include "Commons.h"

int main(int argc, const char * argv[]) {

  // if (argc != 3) {
	// 	fprintf(stderr, "Wrong number of Arguments: Missing prefix\n\n");
	// 	exit(MISSING_ARGS);
	// }

  int n = 1;

  pid_t child;
  int nextPipes[2];
  nextPipes[0] = dup(0);
  nextPipes[1] = dup(1);

  // int bytes;
  // char c;
  // const char* pre = argv[1];
  // printf("%s", pre);

  for (int i = 0; i < n; ++i) {

    if ((child = fork()) == 0) {

      int pip[2], temp;
      if (pipe(pip) < 0) {
        fprintf(stderr, "MAIN: Cannot open pipe\n");
        exit(PIPE_CREATION_ERROR);
      }

      fprintf(stderr, "DEBUG: CHILD: created pip in=%d, out=%d\n", pip[0], pip[1]);

      // replace stdin pipe
      close(0);
      if ((temp = dup(nextPipes[0])) != 0) {
        fprintf(stderr, "CHILD PROC: Expected input pipe to be 0, got %d \n", temp);
        exit(PIPE_CREATION_ERROR);
      } else {
        fprintf(stderr, "DEBUG: CHILD: dup read %d from %d\n", temp, nextPipes[0]);
      }

      // set read pip for next proccess
      close(nextPipes[0]);
      nextPipes[0] = dup(pip[0]);
      fprintf(stderr, "DEBUG: CHILD: dup next read %d from %d\n", nextPipes[0], pip[0]);

      // replace stdout pipe
      fprintf(stderr, "DEBUG: CHILD: about to close 1\n");
      close(1);
      fprintf(stderr, "DEBUG: CHILD: closed 1\n");
      int writeTo = (i == n-1) ? nextPipes[1] : pip[1];
      fprintf(stderr, "DEBUG: CHILD: about to dup 1 from %d\n", writeTo);
      if ((temp = dup(writeTo)) != 1) {
        fprintf(stderr, "CHILD PROC: Expected output pipe to be 1, got %d\n", temp);
        exit(PIPE_CREATION_ERROR);
      } else {
        fprintf(stderr, "DEBUG: CHILD: dup write %d from %d\n", temp, writeTo);
      }

      // Close temp pipes
      fprintf(stderr, "DEBUG: CHILD:close pip in %d\n", pip[0]);
      close(pip[0]);
      fprintf(stderr, "DEBUG: CHILD:close pip out %d\n", pip[1]);
      close(pip[1]);

      /// TODO: execve writer
      char* args[3];
      args[0] = "writer";
      args[1] = "A";
      args[2] = NULL;
      execve(args[0], args, NULL);
      fprintf(stderr, "*** ERROR: *** EXEC of %s FAILED\n", args[0]);
      exit(1);

    } else { // Parent
      printf("MAIN: Created new proccess: %d\n", child);
    }

  }

  // while ((bytes = read(0, &c, sizeof(c))) > 0) {
  //   printf("%c", c);
  //   if (c == '\n')
  //     printf("%s", pre);
  // }
  //
  // if (bytes == -1) {
  //     printf("MAIN: Error reading from pipe");
  //     exit(PIPE_READ_ERROR);
  // }

  // No need for anymore pipes
  close(0);
  close(1);
  return 0;
}
