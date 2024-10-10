#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    FILE* br = fopen("test.txt", "r");

    assert(ftell(br) == 0);
    fseek(br, 0, SEEK_END);
    assert(ftell(br) == 110);
    assert(getc(br) == EOF);

    fseek(br, 0, SEEK_SET);
    assert(ftell(br) == 0);

    char buf[256];
    assert(fgets(buf, 256, br));
    buf[strcspn(buf, "\n")] = '\0';
    assert(!strcmp("CQKYCBGnoJ", buf));
    assert(ftell(br) == 11); // consumes the \n as well as the 10 chars

    assert(fgets(buf, 256, br));
    buf[strcspn(buf, "\n")] = '\0';
    assert(!strcmp("UTuGUgYJBI", buf));
    assert(ftell(br) == 22); // consumes the \n as well as the 10 chars

    assert(getc(br) == 'V');
    assert(ftell(br) == 23);

    assert(fread(buf, 1, 20, br) == 20);
    assert(!strncmp("pGoAshMif\nRClfUUZbTN", buf, 20));
    assert(ftell(br) == 43);

    size_t read = fread(buf, 1, 255, br);
    buf[read] = '\0';
    assert(!strcmp("\nQMwzobCazv\nKjtSbQfkYZ\nfxCPNqvqDh\nRBSAlnFqPb\ndIqyfhUdd"
                   "S\nNPexpHDjRK\n",
                   buf));
    assert(ftell(br) == 110);

    assert(getc(br) == -1);
    assert(ftell(br) == 110);
    assert(!fgets(buf, 256, br));
    assert(ftell(br) == 110);

    fseek(br, 0, SEEK_SET);
    assert(ftell(br) == 0);
    fseek(br, -7, SEEK_END);
    assert(ftell(br) == 103);

    assert(fgets(buf, 256, br));
    buf[strcspn(buf, "\n")] = '\0';
    assert(ftell(br) == 110);
    assert(!strcmp("pHDjRK", buf));

    fseek(br, 0, SEEK_SET);
    assert(ftell(br) == 0);
    fseek(br, -7, SEEK_END);
    assert(ftell(br) == 103);

    assert(fread(buf, 1, 256, br) == 7);
    assert(ftell(br) == 110);
    buf[7] = '\0';
    assert(!strcmp("pHDjRK\n", buf));

    fseek(br, 400, SEEK_END);
    printf("%zd\n", ftell(br));

    fclose(br);

    puts("success");
}
