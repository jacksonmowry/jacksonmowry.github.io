#include "graph_matrix.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    int V = atoi(argv[1]);
    int E = atoi(argv[2]);

    Graph G = graph_init(V, E);

    int v, w;
    int edge_count = 0;
    while (scanf("%d %d", &v, &w) == 2) {
        graph_insert_edge(G, (Edge){.v = v, .w = w});
        edge_count++;
    }

    graph_print(G);
    graph_print_edges(G);

    graph_destroy(G);
}
