#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

typedef struct node {
  int value;
  struct node* adjacency[];
} Node;

#define DISP_PERROR 1
#define DISP_USAGE 2
#define NEXT_FLAG 3

void terminateGracefully(char* actionName, int flags) {
  // remember to do your own cleanup
  // before you call this! In particular
  // clean up your open file descriptors,
  // etc.
  fprintf(stderr, "\n*** ERROR *** failed while ");
  if (flags & DISP_PERROR) {
    perror(actionName);
  } else {
    fprintf(stderr, "%s", actionName);
  }
  if (flags & DISP_USAGE) {
    char message[] = "\n\nGRAPH CENTRALITY\n\nA simple program to load a graph and \
compute its centrality using various techniques.\n\
\nUSAGE\n\n\
graph <graphfile>\n\n\
<graphfile> should be a file containing a graph expressed as a \
blank separated adjacency list, one edge per line, e.g.:\n\n\
--- graph.in ---\n\
1 2\n\
1 3\n\
2 3\n\
3 4\n\
3 5\n\
5 1\n\
--- graph.in ---\n\n\
input like this should produce the following graph:\n\
1 -- 2\n\
|\\   |\n\
| -- 3 -- 4\n\
\\    |\n\
 ----5\n\
The program works best when node indices are small.\n\n";
  fprintf(stderr, "%s", message);
  }
  exit(-1);
}

int graphCap = 10;

int graphSize = 0;

int* adjacencyCounts; 

#define readBuffSize 10000
#define lineBuffSize 1000

char readBuffer[readBuffSize];
char lineBuffer[lineBuffSize];

int lineFeedAt = 0;

void solveLine() {
  lineBuffer[lineFeedAt] = '\0';
  char* left = strtok(lineBuffer, " ");
  char* right = strtok(0, " ");
  printf("%s ", left);
  printf("%s\n", right);
  lineFeedAt = -1;
}

Node* loadGraph(char filename[]) {
  int fileDes = open(filename, O_RDONLY);
  if (fileDes < 0) {
    terminateGracefully("opening a file", DISP_USAGE | DISP_PERROR);
  }
  int readStatus;
  while ((readStatus = read(fileDes, readBuffer, sizeof(char))) != 0) {
    for (int i = 0; i < readStatus; i++, lineFeedAt++) {
      lineBuffer[lineFeedAt] = readBuffer[i];
      if (readBuffer[i] == '\n') {
        solveLine();
      }
      if (lineFeedAt + 1 == lineBuffSize) {
        terminateGracefully("too many characters per line. Reduce your input size.", 0);
      }
    }
    if (readStatus < 0) {
      terminateGracefully("reading a file", DISP_USAGE | DISP_PERROR);
    }
  }
  close(fileDes);
  Node* root = malloc(sizeof(Node));
  return root;
} 

int main(int argc, char* argv[]) {
  if (argc == 1) {
    terminateGracefully("opening a file. You didn't specify the graph file.", DISP_USAGE);
  }
  Node* root = loadGraph(*(argv + 1));
  //printf("%d", root->value);
  return 0;
}
