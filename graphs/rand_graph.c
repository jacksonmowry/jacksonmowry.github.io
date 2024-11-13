#include "graph.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    int V = atoi(argv[1]);
    int E = atoi(argv[2]);
    Graph G = graph_rand(V, E);
    if (V < 20) {
        graph_print(G);
    } else {
        printf("%d verticies, %d edges\n", V, E);
    }

    printf("%d component(s)\n", graph_connected_components(G));
}
