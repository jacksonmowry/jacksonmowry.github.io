#ifndef GRAPH_H_
#define GRAPH_H_
#include <stdbool.h>

typedef struct {
    int v;
    int w;
} Edge;
Edge edge_init(int v1, int v2);

typedef struct graph* Graph;
Graph graph_init(int max_verticies, int max_edges);
Graph graph_from_edges(Edge[], int);
void graph_insert_edge(Graph, Edge);
void graph_remove_edge(Graph, Edge);
int graph_edges(Edge[], Graph G);
bool graph_edge(Graph G, Edge E);
Graph graph_rand(int verticies, int edges);
void graph_print(Graph);
void graph_print_edges(Graph);
int graph_connected_components(Graph);
Graph graph_copy(Graph);
void graph_destroy(Graph);

#endif // GRAPH_H_
