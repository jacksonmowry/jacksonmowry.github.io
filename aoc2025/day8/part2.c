#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int dcmp(const void *x, const void *y) {
    Pairing *a = (Pairing *)x;
    Pairing *b = (Pairing *)y;
    if (a->dist < b->dist) {
        return -1;
    } else if (b->dist < a->dist) {
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

    Pairing closest[graph_size * graph_size];
    memset(closest, 0, graph_size * graph_size * sizeof(closest[0]));
    size_t num_closest = 0;

    for (size_t i = 0; i < graph_size; i++) {
        Point a = graph[i].p;
        for (size_t j = i + 1; j < graph_size; j++) {
            Point b = graph[j].p;

            double dist =
                sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) +
                     (a.z - b.z) * (a.z - b.z));

            closest[num_closest++] = (Pairing){
                .i = i,
                .j = j,
                .dist = dist,
            };
        }
    }

    qsort(closest, num_closest, sizeof(closest[0]), dcmp);

    bool fully_connected = 0;
    size_t connection_index = 0;
    while (!fully_connected && connection_index < num_closest) {
        // Make closest connection, check graph size
        Node *a = &graph[closest[connection_index].i];
        Node *b = &graph[closest[connection_index].j];

        add_edge(a, b);
        add_edge(b, a);

        size_t largest = 0;
        for (size_t i = 0; i < graph_size; i++) {
            Node *edge_list[10000];
            size_t num_edges = 0;

            graph_trav(graph + i, edge_list, &num_edges);
            qsort(edge_list, num_edges, sizeof(*edge_list), cmp);

            if (num_edges == graph_size) {
                printf("Found size %zu with leader_node 0x%p\n", num_edges,
                       edge_list[0]);
                printf("Answer %ld\n", a->p.x * b->p.x);
                exit(0);
            } else if (num_edges > 1) {
                if (num_edges > largest) {
                    largest = num_edges;
                }
                continue;
            }
        }

        /* printf("Only found largest subgraph of %zu nodes\n", largest); */
        for (size_t i = 0; i < graph_size; i++) {
            graph[i].visited = false;
        }

        connection_index++;
    }
}
