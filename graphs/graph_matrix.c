#include "graph_matrix.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct graph {
    int verticies;
    bool** matrix;
} graph;

bool* graph_at(Graph G, int row, int col) {
    if (col > row) {
        int tmp = col;
        col = row;
        row = tmp;
    }
    return &G->matrix[row][col];
}

Graph graph_init(int max_verticies, int max_edges) {
    Graph G = calloc(1, sizeof(graph));
    G->verticies = max_verticies;
    G->matrix = calloc(max_verticies * max_verticies, sizeof(bool*));
    for (int i = 0; i < max_verticies; i++) {
        for (int j = 0; j < i + 1; j++) {
            printf("0 ");
        }
        putchar('\n');
        G->matrix[i] = calloc(i + 1, sizeof(bool));
    }

    return G;
}

Graph graph_from_edges(Edge* edges, int num_edges) {
    int max_vert = 0;

    for (int i = 0; i < num_edges; i++) {
        if (edges[i].v > max_vert || edges[i].w > max_vert) {
            max_vert = edges[i].v > edges[i].w ? edges[i].v : edges[i].w;
        }
    }

    Graph G = graph_init(max_vert + 1, 0);

    for (int i = 0; i < num_edges; i++) {
        graph_insert_edge(G, edges[i]);
    }

    return G;
}

void graph_insert_edge(Graph G, Edge E) {
    if (E.w > E.v) {
        *graph_at(G, E.v, E.w) = true;
    } else {
        *graph_at(G, E.w, E.v) = true;
    }
}

void graph_remove_edge(Graph G, Edge E) {
    if (E.w > E.v) {
        *graph_at(G, E.v, E.w) = false;
    } else {
        *graph_at(G, E.w, E.v) = false;
    }
}

int graph_edges(Edge* edges, Graph G) {
    int edge_count = 0;
    for (int i = 0; i < G->verticies; i++) {
        for (int j = 0; j < G->verticies; j++) {
            if (graph_at(G, i, j)) {
                if (edges) {
                    edges[edge_count++] = (Edge){.w = i, .v = j};
                } else {
                    edge_count++;
                }
            }
        }
    }

    return edge_count;
}

bool graph_edge(Graph G, Edge E) {
    if (E.w > E.v) {
        int tmp = E.w;
        E.w = E.v;
        E.v = tmp;
    }
    return graph_at(G, E.v, E.w);
}

void graph_print(Graph G) {
    for (int i = 0; i < G->verticies; i++) {
        for (int j = 0; j < G->verticies; j++) {
            printf("%d ", *graph_at(G, i, j));
        }
        putchar('\n');
    }
}

void graph_print_edges(Graph G) {
    int edge_count = graph_edges(NULL, G);
    Edge edges[edge_count];
    graph_edges(edges, G);

    for (int i = 0; i < edge_count; i++) {
        printf("%d-%d\n", edges[i].v, edges[i].w);
    }
}

void graph_destroy(Graph G) {
    for (int i = 0; i < G->verticies; i++) {
        free(G->matrix[i]);
    }
    free(G->matrix);
    free(G);
}
