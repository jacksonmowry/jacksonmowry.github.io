#include <smmintrin.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <x86intrin.h>

char row_major_board[16][16] = {};
char column_major_board[16][16] = {};
char diag_tl_to_br[16] = {};
char diag_tr_to_bl[16] = {};

char os[16] = {'O', 'O', 'O', 'O', 'O', 'O', 'O', 'O',
               'O', 'O', 'O', 'O', 'O', 'O', 'O', 'O'};
char xs[16] = {'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
               'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'};

bool move(char player, int x, int y) {
  if (row_major_board[x][y] != '\0') {
    return false;
  }
  row_major_board[x][y] = player;
  column_major_board[y][x] = player;

  if (x == y) {
    diag_tl_to_br[x] = player;
  }

  if (abs(x - y) == 2 || (x == 7 && y == 7)) {
    diag_tr_to_bl[x] = player;
  }

  return true;
}

// Tests to see if every char in the vec is the same, which with the data stored
// how it is works very efficiently
bool test_vec(__m128i vector, int turn) {
  if (_mm_test_all_zeros((__m128i){}, vector)) {
    return false;
  }

  __m128i result = _mm_xor_si128(vector, *(__m128i *)(turn % 2 ? os : xs));

  return _mm_test_all_zeros((__m128i){}, result);
}

bool test_iter(char *row, int turn) {
  char first = turn % 2 ? 'X' : 'O';
  for (int i = 0; i < 16; i++) {
    if (row[i] != first) {
      return false;
    }
  }

  return true;
}

bool vectorized = false;

int main(int argc) {
  if (argc == 2) {
    vectorized = true;
  }
  int turn = 0;

  double times[256] = {};

  while (turn < 256) {
    printf("=============================\n");
    for (size_t i = 0; i < 16; i++) {
      for (size_t j = 0; j < 16; j++) {
        printf("%c ", row_major_board[i][j] ? row_major_board[i][j] : ' ');
      }
      printf("\n");
    }
    printf("=============================\n\n");

    int x;
    int y = rand() % 16;
    do {
      x = rand() % 16;
      y = rand() % 16;
    } while (!move(turn % 2 ? 'X' : 'O', x, y));

    __m128i *row_major = (__m128i *)row_major_board;
    __m128i *col_major = (__m128i *)column_major_board;
    __m128i *tl_to_br = (__m128i *)diag_tl_to_br;
    __m128i *tr_to_bl = (__m128i *)diag_tr_to_bl;

    clock_t start = clock();
    if (vectorized) {
      for (size_t i = 0; i < 16; i++) {
        if (test_vec(row_major[i], turn)) {
          printf("%c won on row %zu\n", turn % 2 ? 'X' : 'O', i);
          exit(0);
        }
        if (test_vec(col_major[i], turn)) {
          printf("%c won on col %zu\n", turn % 2 ? 'X' : 'O', i);
          exit(0);
        }
      }

      if (test_vec(tl_to_br[0], turn)) {
        printf("%c won on diag from tl to br\n", turn % 2 ? 'X' : 'O');
        exit(0);
      }
      if (test_vec(tr_to_bl[0], turn)) {
        printf("%c won on diag from tr to bl\n", turn % 2 ? 'X' : 'O');
        exit(0);
      }
    } else {
      for (size_t i = 0; i < 16; i++) {
        if (test_iter(row_major_board[i], turn)) {
          printf("%c won on row %zu\n", turn % 2 ? 'X' : 'O', i);
          exit(0);
        }
        if (test_iter(column_major_board[i], turn)) {
          printf("%c won on col %zu\n", turn % 2 ? 'X' : 'O', i);
          exit(0);
        }
      }

      if (test_iter(diag_tl_to_br, turn)) {
        printf("%c won on diag from tl to br\n", turn % 2 ? 'X' : 'O');
        exit(0);
      }
      if (test_iter(diag_tr_to_bl, turn)) {
        printf("%c won on diag from tr to bl\n", turn % 2 ? 'X' : 'O');
        exit(0);
      }
    }
    times[turn] = ((double)(clock() - start)) / CLOCKS_PER_SEC;

    turn++;

    printf("looping for turn %d!\n", turn);

    usleep(100000);
  }

  printf("Cats game!\n");

  double accumulator = 0;
  for (int i = 0; i < 256; i++) {
    accumulator += times[i];
  }

  printf("Average time: %f seconds\n", accumulator / 256);
}
