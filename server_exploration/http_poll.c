#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8069
#define MAX_CLIENTS 8192

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

int setup_server(short port, int backlo);

int main() {
  signal(SIGPIPE, SIG_IGN);
  char buffer[BUFFER_SIZE];
  bool server_running = true;
  bool close_conn;
  bool compress_array = false;
  int server_socket = setup_server(PORT, MAX_CLIENTS);
  int client_socket;

  struct pollfd fds[MAX_CLIENTS];
  int nfds = 1, current_size = 0;

  memset(fds, 0, sizeof(fds));

  fds[0].fd = server_socket;
  fds[0].events = POLLIN;

  do {
    int err = poll(fds, nfds, -1);
    if (err == -1) {
      perror("poll");
      exit(1);
    }

    current_size = nfds;
    for (int i = 0; i < current_size; i++) {
      // Find the fd with POLLIN
      if (fds[i].events == 0) {
        continue;
      }

      // Server socket, new connection to accept
      if (fds[i].fd == server_socket) {
        // pull connections off of the listen queue
        do {
          client_socket = accept(server_socket, NULL, NULL);
          if (client_socket < 0) {
            break;
          }

          fds[nfds].fd = client_socket;
          fds[nfds].events = POLLIN;
          nfds++;
        } while (client_socket != -1);
      } else {
        // Not the server socket, Recieve messge from client
        close_conn = false;

        do {
          int err = recv(fds[i].fd, buffer, sizeof(buffer), 0);
          if (err < 0) {
            break;
          }
          if (err == 0) {
            close_conn = true;
            break;
          }

          char *method = strtok(buffer, " ");
          char *path = strtok(NULL, " ");
          path++;
          char *filepath = realpath(path, NULL);

          struct stat st;
          stat(filepath, &st);

          size_t header_length =
              snprintf(buffer, sizeof(buffer) - 1,
                       "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: "
                       "text/plain\r\nContent-Length: %ld\r\n\r\n",
                       st.st_size);

          err = send(fds[i].fd, buffer, header_length, 0);
          if (err < 0) {
            free(filepath);
            close_conn = true;
            break;
          }

          int fd = open(filepath, O_RDONLY);
          if (fd == -1) {
            /* perror("open"); */
            exit(1);
          }
          free(filepath);

          err = sendfile(fds[i].fd, fd, NULL, st.st_size);
          if (err == -1) {
            /* perror("sendfile"); */
            close_conn = true;
            break;
          }

          err = close(fd);
          if (err == -1) {
            /* perror("close"); */
            close_conn = true;
            break;
          }

        } while (true);

        if (close_conn) {
          int err = close(fds[i].fd);
          if (err == -1) {
            /* perror("close"); */
          }
          fds[i].fd = -1;
          compress_array = true;
        }
      }
    }

    if (compress_array) {
      compress_array = false;
      for (int i = 0; i < nfds; i++) {
        if (fds[i].fd == -1) {
          for (int j = i; j < nfds - 1; j++) {
            fds[j].fd = fds[j + 1].fd;
          }
          i--;
          nfds--;
        }
      }
    }
  } while (server_running);

  for (int i = 0; i < nfds; i++) {
    if (fds[i].fd >= 0) {
      close(fds[i].fd);
    }
  }
  puts("are we getting here?");
  return 0;
}

int setup_server(short port, int backlog) {
  int server_socket, client_socket, addr_size;
  SA_IN server_addr;

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  assert(server_socket >= 0);

  server_addr = (SA_IN){.sin_family = AF_INET,
                        .sin_addr.s_addr = INADDR_ANY,
                        .sin_port = htons(port)};

  int opt_val = 1;
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

  ioctl(server_socket, FIONBIO, &opt_val);

  int err = bind(server_socket, (SA *)&server_addr, sizeof(server_addr));
  assert(err != -1);

  err = listen(server_socket, backlog);
  assert(err != -1);

  return server_socket;
}
