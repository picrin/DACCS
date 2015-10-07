CC=clang
CFLAGS=-Wall -pedantic -std=c99 -g
graph: graph.c
	$(CC) $(CFLAGS) graph.c -o graph
	
