#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Point {
    long x;
    long y;
    long z;
} Point;

typedef struct Node {
    Point p;
    struct Node **edges;
    size_t num_edges;
    bool visited;
    size_t size;
} Node;

typedef struct Pairing {
    size_t i;
    size_t j;
    double dist;
} Pairing;

void add_edge(Node *n, Node *m) {
    if (n->num_edges == 0) {
        n->edges = malloc(sizeof(*n->edges));
        n->num_edges = 1;
    } else {
        n->edges = realloc(n->edges, ((n->num_edges + 1) * sizeof(*n->edges)));
        n->num_edges++;
    }

    n->edges[n->num_edges - 1] = m;
}

void graph_trav(Node *n, Node **edge_list, size_t *num_edges) {
    if (n->visited) {
        return;
    }

    bool exists = false;
    for (size_t i = 0; i < *num_edges; i++) {
        if (edge_list[i] == n) {
            exists = true;
        }

        break;
    }

    if (!exists) {
        edge_list[*num_edges] = n;
        *num_edges = *num_edges + 1;
    }

    n->visited = true;

    for (size_t i = 0; i < n->num_edges; i++) {
        graph_trav(n->edges[i], edge_list, num_edges);
    }
}

int cmp(const void *a, const void *b) {
    if (a < b) {
        return -1;
    } else if (b < a) {
        return 1;
    } else {
        return 0;
    }
}

int main() {
    Node *graph = malloc(10 * sizeof(*graph));
    size_t graph_size = 0;
    size_t graph_cap = 10;

    char buf[256];
    while (fgets(buf, 256, stdin)) {
        if (graph_size >= graph_cap) {
            graph = realloc(graph, 2 * graph_cap * sizeof(*graph));
            graph_cap *= 2;
        }
        long x;
        long y;
        long z;
        if (3 != sscanf(buf, "%ld,%ld,%ld", &x, &y, &z)) {
            fprintf(stderr, "Failed to read coords\n");
            exit(1);
        }

        graph[graph_size++] = (Node){.p = (Point){.x = x, .y = y, .z = z}};
    }

    // Create adjacency matrix of distance between points

    double mat[20][20] = {0};
    for (size_t i = 0; i < graph_size; i++) {
        Point a = graph[i].p;
        for (size_t j = i; j < graph_size; j++) {
            Point b = graph[j].p;

            mat[i][j] =
                sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) +
                     (a.z - b.z) * (a.z - b.z));
        }
    }

    Pairing closest[10] = {0};
    size_t num_closest = 0;
    double smallest = 1000000;

    for (size_t total = 0; total < 10; total++) {
        for (size_t i = 0; i < graph_size; i++) {
            for (size_t j = i; j < graph_size; j++) {
                if (mat[i][j] < smallest && mat[i][j] != 0) {
                    closest[num_closest] =
                        (Pairing){.i = i, .j = j, .dist = mat[i][j]};
                    smallest = mat[i][j];
                }
            }
        }
        smallest = 1000000000;
        mat[closest[num_closest].i][closest[num_closest].j] = 1000000000;
        num_closest++;
    }

    for (int i = 0; i < num_closest; i++) {
        /* printf("Connecting: (%ld, %ld, %ld) and (%ld, %ld, %ld)\n", */
        /*        graph[closest[i].i].p.x, graph[closest[i].i].p.y, */
        /*        graph[closest[i].i].p.z, graph[closest[i].j].p.x, */
        /*        graph[closest[i].j].p.y, graph[closest[i].j].p.z); */

        Node *a = &graph[closest[i].i];
        Node *b = &graph[closest[i].j];

        add_edge(a, b);
        add_edge(b, a);
    }

    for (size_t i = 0; i < graph_size; i++) {
        Node *edge_list[10000];
        size_t num_edges = 0;

        graph_trav(graph + i, edge_list, &num_edges);
        qsort(edge_list, num_edges, sizeof(*edge_list), cmp);

        if (num_edges > 1) {
            printf("Found size %zu with leader_node 0x%p\n", num_edges,
                   edge_list[0]);
        }
    }
}
