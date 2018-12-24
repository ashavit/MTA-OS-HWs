#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>    // execvp(), pipe()
#include <fcntl.h>
#include "Commons.h"

pid_t child;
int nextPipes[2];
char* writerArgs[3];
const char* outputFile;

void initializeMainVars();
void executeWriterProgram(int progNum);

int main(int argc, const char * argv[]) {

  if (argc != 3) {
		fprintf(stderr, "Wrong number of Arguments: Missing prefix\n\n");
		exit(MISSING_ARGS);
	}

  int n = atoi(argv[1]);
  outputFile = argv[2];
  initializeMainVars();

  for (int i = 0; i < n; ++i) {

      int pip[2], temp;
      if (pipe(pip) < 0) {
        fprintf(stderr, "MAIN: Cannot open pipe\n");
        exit(PIPE_CREATION_ERROR);
      }

    if ((child = fork()) == 0) {

      // replace stdin pipe
      close(0);
      if ((temp = dup(nextPipes[0])) != 0) {
        fprintf(stderr, "CHILD PROC: Expected input pipe to be 0, got %d \n", temp);
        exit(PIPE_CREATION_ERROR);
      }

      // replace stdout pipe
      close(1);
      int writeTo = (i == n-1) ? nextPipes[1] : pip[1];
      if ((temp = dup(writeTo)) != 1) {
        fprintf(stderr, "CHILD PROC: Expected output pipe to be 1, got %d\n", temp);
        exit(PIPE_CREATION_ERROR);
      }

      // Close temp pipes
      close(pip[0]);
      close(pip[1]);
      close(nextPipes[0]);
      close(nextPipes[1]);

      // Execute writer
      executeWriterProgram(i);

    } else { // Parent
      // printf("MAIN: Created new proccess: %d\n", child);

      // set read pip for next proccess
      close(nextPipes[0]);
      nextPipes[0] = dup(pip[0]);

      close(pip[0]);
      close(pip[1]);
    }
  }

  // Wait for all child programs to end
  for (int i = 0; i < n; ++i) {
    wait(NULL);
  }

  // Close output file with new line
  write(nextPipes[1], "\n", 1);
  close(nextPipes[0]);
  close(nextPipes[1]);

  return 0;
}

void initializeMainVars() {
  writerArgs[0] = "writer";
  writerArgs[1] = NULL;
  writerArgs[2] = NULL;

  nextPipes[0] = dup(0);
  nextPipes[1] = open(outputFile, O_WRONLY | O_APPEND | O_CREAT, 0644);

  if (nextPipes[0] < 0 || nextPipes[1] < 0) {
    fprintf(stderr, "MAIN: Cannot open pipe\n");
    exit(PIPE_CREATION_ERROR);
  }
}

void executeWriterProgram(int progNum) {
  char prefix[10];
  sprintf(prefix, "%c", 'A' + progNum);
  writerArgs[1] = prefix;

  execve(writerArgs[0], writerArgs, NULL);
  fprintf(stderr, "*** ERROR: *** EXEC of %s FAILED\n", writerArgs[0]);
  exit(LOAD_PROGRAM_ERROR);
}
