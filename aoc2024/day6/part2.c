#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

char map[130][135] = {0};
const int dirs[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

typedef struct Work {
  _Atomic int count;
  _Atomic int pos;
  int total;
  int width;
  int height;
  int x;
  int y;
  int dir;
} Work_t;

void *worker(void *arg) {
  Work_t *w = (Work_t *)arg;

  while (true) {
    int job = w->pos++;

    if (job >= w->total) {
      break;
    }

    int i = job / w->width;
    int j = job % w->width;

    if (map[i][j] != '.') {
      continue;
    }

    /* thread_map[i][j] = '#'; */

    int inner_x = w->x;
    int inner_y = w->y;
    int inner_dir = w->dir;
    int inner_count = 0;

    while (inner_x >= 0 && inner_x < w->width && inner_y >= 0 &&
           inner_y < w->height) {
      if (inner_count >= 6240) {
        w->count += 1;
        break;
      }
      if (inner_y + dirs[inner_dir][0] < 0 ||
          inner_y + dirs[inner_dir][0] >= w->height ||
          inner_x + dirs[inner_dir][1] < 0 ||
          inner_x + dirs[inner_dir][1] >= w->width) {
        break;
      }
      if ((inner_y + dirs[inner_dir][0] == i &&
           inner_x + dirs[inner_dir][1] == j) ||
          map[inner_y + dirs[inner_dir][0]][inner_x + dirs[inner_dir][1]] ==
              '#') {
        // rotate 90 degrees clockwise
        inner_dir = (inner_dir + 1) % 4;
      } else {
        inner_count++;
        inner_x += dirs[inner_dir][1];
        inner_y += dirs[inner_dir][0];
      }
    }

    /* thread_map[i][j] = '.'; */
  }

  return NULL;
}

int main() {
  int x = -1;
  int y = -1;
  int dir = -1;

  int width;
  int row = 0;
  while (fgets((char *)&map[row++], 134, stdin)) {
    int cursor = 0;
    char current;
    if (x == -1) {
      while ((current = map[row - 1][cursor++])) {
        if (current == '^') {
          x = cursor - 1;
          y = row - 1;
          dir = 0;
        } else if (current == '>') {
          x = cursor - 1;
          y = row - 1;
          dir = 1;
        } else if (current == 'v') {
          x = cursor - 1;
          y = row - 1;
          dir = 2;
        } else if (current == '<') {
          x = cursor - 1;
          y = row - 1;
          dir = 3;
        }
      }
    }
  }

  width = strlen(map[0]);

  Work_t work = {.count = 0,
                 .pos = 0,
                 .total = (row - 1) * (width),
                 .width = strlen(map[0]),
                 .height = row - 1,
                 .x = x,
                 .y = y,
                 .dir = dir};

  pthread_t tids[8];
  for (int i = 0; i < 8; i++) {
    pthread_create(tids + i, NULL, worker, &work);
  }

  for (int i = 0; i < 8; i++) {
    pthread_join(tids[i], NULL);
  }

  /* for (int i = 0; i < row - 1; i++) { */
  /*   printf("%s", map[i]); */
  /* } */

  printf("%d\n", work.count);
}
