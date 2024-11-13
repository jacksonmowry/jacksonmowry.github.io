#include "graph_list.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct graph {
    Edge* edges;
    int num_edges;
    int max_edges;
    int verticies;
} graph;

Graph graph_init(int max_verticies, int max_edges) {
    Graph G = calloc(1, sizeof(graph));

    G->num_edges = 0;
    G->max_edges = max_edges;
    G->edges = calloc(max_edges, sizeof(Edge));
    G->verticies = max_verticies;

    return G;
}

void graph_insert_edge(Graph G, Edge E) {
    for (int i = 0; i < G->num_edges; i++) {
        Edge e = G->edges[i];
        if (E.w == e.w && E.v == e.v || E.w == e.v && E.v == e.w) {
            return;
        }
    }
    G->edges[G->num_edges++] = E;
}

void graph_remove_edge(Graph G, Edge E) {
    for (int i = 0; i < G->num_edges; i++) {
        Edge e = G->edges[i];
        if (E.w == e.w && E.v == e.v || E.w == e.v && E.v == e.w) {
            G->edges[i] = G->edges[G->num_edges - 1];
            G->num_edges--;
        }
    }
}

bool graph_edge(Graph G, Edge E) {
    for (int i = 0; i < G->num_edges; i++) {
        Edge e = G->edges[i];
        if (E.w == e.w && E.v == e.v || E.w == e.v && E.v == e.w) {
            return true;
        }
    }

    return false;
}

void graph_print(Graph G) {
    for (int i = 0; i < G->verticies; i++) {
        printf("%d: ", i);
        for (int j = 0; j < G->num_edges; j++) {
            if (G->edges[j].v == i || G->edges[j].w == i) {
                printf("%d ",
                       G->edges[i].v == i ? G->edges[i].w : G->edges[i].v);
            }
        }
        putchar('\n');
    }
}

void graph_print_edges(Graph G) {
    for (int i = 0; i < G->num_edges; i++) {
        printf("%d-%d\n", G->edges[i].v, G->edges[i].w);
    }
}

void graph_destroy(Graph G) {
    free(G->edges);
    free(G);
}
