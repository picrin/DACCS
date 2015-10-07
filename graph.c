#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define ADJ_CAP 2
#define GRAPH_CAP 2
#define READ_BUFF_SIZE 10000
#define LINE_BUFF_SIZE 1000
#define CLEANER_SIZE 10

#define DISP_PERROR 1
#define DISP_USAGE 2
#define CLEAN_EXIT 3

typedef struct node {
  int value;
  int adjacencyCap;
  int adjacencySize;
  struct node** adjacency;
} Node;

typedef void (*Cleaner)();

int cleaners_no = 0;
Cleaner cleaners[CLEANER_SIZE];

void registerCleaner(Cleaner cleaner){
  cleaners[cleaners_no] = cleaner;
  cleaners_no++;
}

int graphCap = GRAPH_CAP;
Node** graph;

Node* createNode() {
  Node* node = malloc(sizeof(Node));
  node->adjacency = malloc(ADJ_CAP * sizeof(Node*));
  node->adjacencyCap = GRAPH_CAP;
  node->adjacencySize = 0;
  return node;
}

void destroyNode(Node* node) {
  if (node != NULL) {
    free(node->adjacency);
  }
  free(node);
}

void graphDestroyer() {
  for (int i = 0; i < graphCap; i++) {
    destroyNode(graph[i]);
  }
  free(graph);
}

void terminateGracefully(char* actionName, int flags) {
  // remember to do your own cleanup
  // before you call this! In particular
  // clean up your open file descriptors,
  // etc. All registered cleaners will be
  // called
  for (int i = 0; i < cleaners_no; i++) {
    cleaners[i]();
  }
  if (flags & CLEAN_EXIT) {
    fprintf(stdout, "%s", actionName);
    exit(0);
  }
  fprintf(stderr, "\n*** ERROR *** ");
  if (flags & DISP_PERROR) {
    perror(actionName);
    fprintf(stderr, "\n\n");
  } else {
    fprintf(stderr, "%s\n\n", actionName);
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

void addAdjacency(Node* parent, Node* child) {
  if (parent->adjacencySize == parent->adjacencyCap) {
    parent->adjacencyCap *= 2;
    size_t sizeToRealloc = parent->adjacencyCap * sizeof(Node*);
    parent->adjacency = realloc(parent->adjacency, sizeToRealloc);
    if (parent->adjacency == NULL) {
      terminateGracefully("to many nodes in the adjacency list, couldn't malloc", DISP_PERROR);
    }
    memset(parent->adjacency, 0, sizeToRealloc);
  }
  parent->adjacency[parent->adjacencySize] = child;
  parent->adjacencySize += 1;
}

void connectNodes(Node* node1, Node* node2) {
  addAdjacency(node1, node1);
  addAdjacency(node2, node1);
}

int* adjacencyCounts; 

char readBuffer[READ_BUFF_SIZE];
char lineBuffer[LINE_BUFF_SIZE];

void setUpGraph() {
  graph = calloc(graphCap, sizeof(Node*));

  if (graph == NULL) {
    terminateGracefully("couldn't allocate memory for the graph", DISP_PERROR);
  }
  registerCleaner(graphDestroyer);
}

void growGraph() {
  graphCap = 2*graphCap;
  size_t arrayListSize = graphCap * sizeof(Node*);
  graph = realloc(graph, arrayListSize);
  if (graph == NULL) {
    terminateGracefully("couldn't regrow the graph. Maybe your indexes are too high?", DISP_PERROR);
  }
  memset(graph, 0, arrayListSize);

}

Node* upsertNode(int nodeNumber){
  if (nodeNumber < graphCap) {
    Node* node = graph[nodeNumber];
    if (node != NULL) {
      return node;
    } else {
      node = createNode();
      graph[nodeNumber] = node;
      return node;
    }
  } else {
    growGraph();
    return upsertNode(nodeNumber);
  }
}


int lineFeedAt = 0;

void solveLine() {
  lineBuffer[lineFeedAt] = '\0';
  char* left = strtok(lineBuffer, " ");
  char* right = strtok(0, " ");
  //printf("%s ", left);
  char* leftRightStr[2] = {left, right};
  int leftRightInt[2];
  Node* nodes[2];
  for (int i = 0; i < 2; i++ ){
    int value = (int) strtol(leftRightStr[i], NULL, 10);
    printf("%d", value);
    if (errno == EINVAL) {
      terminateGracefully("your graph needs to be a pair of space delimited integer values per line", DISP_PERROR);
    }
    leftRightInt[i] = value;
    nodes[i] = upsertNode(value);
  }
  connectNodes(nodes[0], nodes[1]);
  lineFeedAt = -1;
}

void loadGraph(char filename[]) {
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
      if (lineFeedAt + 1 == LINE_BUFF_SIZE) {
        close(fileDes);
        terminateGracefully("too many characters per line. Reduce your input's line size.", 0);
      }
    }
    if (readStatus < 0) {
      close(fileDes);
      terminateGracefully("reading a graph file", DISP_USAGE | DISP_PERROR);
    }
  }
  close(fileDes);
} 

int main(int argc, char* argv[]) {
  if (argc == 1) {
    terminateGracefully("opening a graph file. You didn't specify the graph file.", DISP_USAGE);
  }
  setUpGraph();
  loadGraph(*(argv + 1));
  //printf("%d", root->value);
  terminateGracefully("bye bye", CLEAN_EXIT);
}
