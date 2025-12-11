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

uint16_t idx(char name[3]) {
    uint16_t index = 0;
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

size_t dfs(uint16_t n, uint16_t end, size_t *path_counts) {
    if (n == end) {
        return 1;
    }

    if (path_counts[n] == (size_t)-1) {
        // Not yet explored
        size_t count = 0;
        for (size_t i = 0; i < graph[n].num_edges; i++) {
            count += dfs(graph[n].edges[i].dest, end, path_counts);
        }

        path_counts[n] = count;
        return count;
    } else {
        // Explored, return previous count
        return path_counts[n];
    }
}

int main() {
    char buf[1024];
    while (fgets(buf, sizeof(buf), stdin)) {
        char name[3];
        memcpy(name, buf, 3);
        uint16_t index = idx(name);

        char *ptr = strtok(buf + 5, " \n");
        do {
            memcpy(name, ptr, 3);
            uint16_t edge_index = idx(name);

            graph[index].edges[graph[index].num_edges++].dest = edge_index;
        } while ((ptr = strtok(NULL, " \n")));
    }

    size_t svr = idx("svr");
    size_t fft = idx("fft");
    size_t dac = idx("dac");

    size_t sum = 0;

    size_t path_counts[MAX_NODE] = {0};
    memset(path_counts, -1, MAX_NODE * sizeof(size_t));
    // svr -> fft -> dac -> out
    size_t first = dfs(svr, fft, path_counts);
    memset(path_counts, -1, MAX_NODE * sizeof(size_t));
    size_t second = dfs(fft, dac, path_counts);
    memset(path_counts, -1, MAX_NODE * sizeof(size_t));
    size_t third = dfs(dac, OUT, path_counts);

    sum += first * second * third;
    printf("Total: (%zu * %zu * %zu) %zu\n", first, second, third,
           first * second * third);

    memset(path_counts, -1, MAX_NODE * sizeof(size_t));
    // svr -> dac -> fft -> out
    first = dfs(svr, dac, path_counts);
    memset(path_counts, -1, MAX_NODE * sizeof(size_t));
    second = dfs(dac, fft, path_counts);
    memset(path_counts, -1, MAX_NODE * sizeof(size_t));
    third = dfs(fft, OUT, path_counts);

    sum += first * second * third;
    printf("Total: (%zu * %zu * %zu) %zu\n", first, second, third,
           first * second * third);

    printf("Sum: %zu\n", sum);
}
