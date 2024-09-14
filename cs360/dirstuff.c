#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void print_cl(mode_t mode) {
  if (S_ISDIR(mode)) {
    printf("d");
  } else if (S_ISLNK(mode)) {
    printf("l");
  } else {
    printf("-");
  }

  for (int i = 6; i >= 0; i -= 3) {
    putchar(mode & (1 << (i + 2)) ? 'r' : '-');
    putchar(mode & (1 << (i + 1)) ? 'w' : '-');
    putchar(mode & (1 << (i)) ? 'x' : '-');
  }
}

int main() {
  struct stat st;
  DIR *dir;
  struct dirent *dent;

  dir = opendir(".");
  if (!dir) {
    perror("opendir");
    return 1;
  }

  while ((dent = readdir(dir)) != NULL) {
    if (strcmp(dent->d_name, ".") && strcmp(dent->d_name, "..")) {
      printf("%s\n", dent->d_name);
    }
  }

  closedir(dir);
}
