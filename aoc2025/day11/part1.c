#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct Edge {
    uint16_t dest;
} Edge;

typedef struct Node {
    Edge edges[24];
    uint8_t num_edges;
} Node;

#define MAX_NODE 17578
#define YOU 17576
#define OUT 17577

size_t idx(char name[3]) {
    size_t index = 0;
    if (!strncmp("you", name, 3)) {
        index = YOU;
    } else if (!strncmp("out", name, 3)) {
        index = OUT;
    } else {
        index = ((name[0] - 'a') * 26 * 26) + ((name[1] - 'a') * 26) +
                (name[2] - 'a');
    }

    return index;
}

Node graph[17578] = {0};

size_t dfs(uint16_t n, uint16_t end, size_t *path_count) {
    if (n == end) {
        return 1;
    }

    if (path_count[n] != (size_t)-1) {
        return path_count[n];
    }

    size_t count = 0;

    for (size_t i = 0; i < graph[n].num_edges; i++) {
        count += dfs(graph[n].edges[i].dest, end, path_count);
    }

    path_count[n] = count;
    return count;
}

int main() {
    char buf[1024];
    while (fgets(buf, sizeof(buf), stdin)) {
        char name[3];
        memcpy(name, buf, 3);
        size_t index = idx(name);

        char *ptr = strtok(buf + 5, " \n");
        do {
            memcpy(name, ptr, 3);
            size_t edge_index = idx(name);

            graph[index].edges[graph[index].num_edges++].dest = edge_index;
        } while ((ptr = strtok(NULL, " \n")));
    }

    size_t path_count[MAX_NODE];
    memset(path_count, -1, MAX_NODE * sizeof(size_t));

    size_t count = dfs(YOU, OUT, path_count);

    printf("Total: %zu\n", count);
}
