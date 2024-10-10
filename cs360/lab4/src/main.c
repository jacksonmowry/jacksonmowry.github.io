#include "bufread.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    BufReader* br = br_open("test.txt", 20, 10);

    assert(br_tell(br) == 0);
    br_seek(br, 0, SEEK_END);
    assert(br_tell(br) == 110);
    assert(br_getchar(br) == -1);

    br_seek(br, 0, SEEK_SET);
    assert(br_tell(br) == 0);

    char buf[256];
    assert(br_getline(br, buf, 256));
    assert(!strcmp("CQKYCBGnoJ", buf));
    assert(br_tell(br) == 11); // consumes the \n as well as the 10 chars

    assert(br_getline(br, buf, 256));
    assert(!strcmp("UTuGUgYJBI", buf));
    assert(br_tell(br) == 22); // consumes the \n as well as the 10 chars

    assert(br_getchar(br) == 'V');
    assert(br_tell(br) == 23);

    assert(br_read(br, buf, 20) == 20);
    assert(!strncmp("pGoAshMif\nRClfUUZbTN", buf, 20));
    assert(br_tell(br) == 43);

    size_t read = br_read(br, buf, 255);
    buf[read] = '\0';
    assert(!strcmp("\nQMwzobCazv\nKjtSbQfkYZ\nfxCPNqvqDh\nRBSAlnFqPb\ndIqyfhUdd"
                   "S\nNPexpHDjRK\n",
                   buf));
    assert(br_tell(br) == 110);

    assert(br_getchar(br) == -1);
    assert(br_tell(br) == 110);
    assert(!br_getline(br, buf, 256));
    assert(br_tell(br) == 110);

    br_seek(br, 0, SEEK_SET);
    assert(br_tell(br) == 0);
    br_seek(br, -7, SEEK_END);
    assert(br_tell(br) == 103);

    assert(br_getline(br, buf, 256));
    assert(br_tell(br) == 110);
    assert(!strcmp("pHDjRK", buf));

    br_seek(br, 0, SEEK_SET);
    assert(br_tell(br) == 0);
    br_seek(br, -7, SEEK_END);
    assert(br_tell(br) == 103);

    assert(br_read(br, buf, 256) == 7);
    assert(br_tell(br) == 110);
    buf[7] = '\0';
    assert(!strcmp("pHDjRK\n", buf));

    br_seek(br, 400, SEEK_END);
    printf("%zd\n", br_tell(br));

    br_close(br);

    puts("success");
}
