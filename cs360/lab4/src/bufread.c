// Jackson Mowry
// Buffered Reader Lab
// Tue Sep 24 09:48:33 2024
// A buffered reader which supports opening a file an storing a part of the
// contents temporarily in memory to speed up read calls.
// The buffer supports 2 tunable parameters, `capacity` and `fill_trigger`.
// `capacity` sets the total number of bytes which will be buffered, essentially
// how much memory will the buffered reader consume.
// `fill_trigger` sets the minimum number of bytes which will trigger another
// `read` call to fill up the buffer.
#include "bufread.h"
#include "rbuf.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct BufReader {
    struct RingBuffer* rb;
    size_t fill; // The trigger to load all of the data from the file in to the
                 // ring buffer
    int fd;      // The file descriptor which is returned from `open()`
};

// if `ammount` == -1 we will do an autofill to make size==cap
static size_t fill_buffer(struct RingBuffer* rb, ssize_t ammount, int fd) {
    if (ammount == -1) {
        ammount = rb_capacity(rb) - rb_size(rb);
    }
    char temp[ammount];

    size_t actually_read = read(fd, temp, ammount);

    return rb_write(rb, temp, actually_read);
}

static size_t get_cur_pos(const struct BufReader* br) {
    return lseek(br->fd, 0, SEEK_CUR) - rb_size(br->rb);
}

// `br_open` Opens the file given by `path`, creates a new rb with `capacity`
// bytes, and sets the given `fill_trigger`.
// Returns NULL if the file cannot be opened.
// Instantly reads `capacity` bytes so that the first read call will hit the
// buffer
struct BufReader* br_open(const char path[], size_t capacity,
                          size_t fill_trigger) {
    if (capacity <= 0 || fill_trigger <= 0 || fill_trigger >= capacity) {
        fprintf(stderr, "Invalid capacity or fill trigger\n");
        return NULL;
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror(path);
        return NULL;
    }

    struct BufReader* br = malloc(sizeof(struct BufReader));
    if (!br) {
        perror("malloc");
        close(fd);
        return NULL;
    }

    *br = (struct BufReader){
        .rb = rb_new(capacity), .fill = fill_trigger, .fd = fd};

    if (!br->rb) {
        free(br);
        close(fd);
        return NULL;
    }

    fill_buffer(br->rb, capacity, fd);

    return br;
}

// `br_close` closes the buffered reader, frees all resources, and any file
// descriptors. After calling close the BufReader* is no longer valid
void br_close(struct BufReader* br) {
    rb_free(br->rb);
    close(br->fd);
    free(br);
}

// `br_seek` moves the cursor within the buffered file stream. Because th reader
// will only look forward it must be cleared if we move backwards. Moving
// forward just ignores bytes, refilling if `fill_trigger` is hit.
// SEEK_SET = beginning, SEEK_CUR = current pos, SEEK_END = end
void br_seek(struct BufReader* br, ssize_t offset, int whence) {
    size_t cur_pos = get_cur_pos(br);

    off_t new_pos = lseek(br->fd, offset, whence);
    if (new_pos == -1) {
        // some sort of error case?
        perror("lseek");
        return;
    }

    if (new_pos - cur_pos < rb_size(br->rb)) {
        // Ignore new_pos - cur_pos bytes
        rb_ignore(br->rb, new_pos - cur_pos);
        // If we dip below fill trigger we'll need to refill
        if (rb_size(br->rb) <= br->fill) {
            fill_buffer(br->rb, -1, br->fd);
        }
    } else {
        // clear and restart
        rb_clear(br->rb);
        fill_buffer(br->rb, -1, br->fd);
    }
}

// `br_tell` returns the current location of the file pointer in the BufReader,
// or -1 on error.
ssize_t br_tell(const struct BufReader* br) {
    if (!br || lseek(br->fd, 0, SEEK_CUR) == -1) {
        return -1;
    }

    return get_cur_pos(br);
}

// `br_getline` Get an entire line of text from the buffer.
// The line will end at any of the following [EOD, '\n', Size read].
// Will always store a NULL byte at the end of either the read chunk, or the
// last position of the buffer, meaning at most `size`-1 bytes are read.
// '\n' is consumed and not stored in the buffer.
// Returns true if successful, otherwise false.
bool br_getline(struct BufReader* br, char s[], size_t size) {
    if (!br || lseek(br->fd, 0, SEEK_CUR) ==
                   -1) { // Probably another condition for an immediate EOF?
        return false;
    }

    size_t s_cursor = 0;
    while (s_cursor < size - 1 && rb_peek(br->rb) != EOF &&
           rb_peek(br->rb) != '\n') {
        if (!rb_pop(br->rb, &s[s_cursor++])) {
            return false;
        }
        if (rb_size(br->rb) <= br->fill) {
            // do a refill
            fill_buffer(br->rb, -1, br->fd);
        }
    }

    br_getchar(br); // Skip over possible '\n'
    if (rb_size(br->rb) <= br->fill) {
        // do a refill
        fill_buffer(br->rb, -1, br->fd);
    }

    s[s_cursor] = '\0';

    if (s_cursor == 0) {
        return false;
    }

    return true;
}

// `br_getchar` Returns a single char from the buffered reader, type cases it
// into an int, and returns it. Returns -1 type casted to an int if there is
// nothing to read;
int br_getchar(struct BufReader* br) {
    if (!br || lseek(br->fd, 0, SEEK_CUR) == -1) {
        return -1;
    }

    char ret;
    if (!rb_pop(br->rb, &ret)) {
        return -1;
    }

    if (rb_size(br->rb) <= br->fill) {
        // do a refill
        fill_buffer(br->rb, -1, br->fd);
    }

    return ret;
}

// `br_read` reads `max_bytes` into the buffer `dest`. Unlike `br_getline` this
// does not stop at a new line, and it does not add a NULL terminator. Returns
// the actual number of bytes read, may be 0 if there is nothing to read or if
// `max_bytes` is invalid.
// Will switch from buffered reading to direct reading if we exhaust the buffer
size_t br_read(struct BufReader* br, void* dest, size_t max_bytes) {
    if (!br || lseek(br->fd, 0, SEEK_CUR) == -1) {
        return 0;
    }

    size_t in_buf = rb_size(br->rb);
    size_t bytes_read = 0;

    if (in_buf > max_bytes) {
        // Simple read
        bytes_read += rb_read(br->rb, dest, max_bytes);
    } else {
        bytes_read += rb_read(br->rb, dest, rb_size(br->rb));
        max_bytes -= bytes_read;

        bytes_read += read(br->fd, &((char*)dest)[bytes_read], max_bytes);
    }

    // Check refill
    if (rb_size(br->rb) <= br->fill) {
        fill_buffer(br->rb, -1, br->fd);
    }
    return bytes_read;
}
