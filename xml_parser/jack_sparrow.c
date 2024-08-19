#include "xml_parser.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    struct stat sb;

    int fd = open("example.xml", O_RDONLY);
    fstat(fd, &sb);
    char* file = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    streaming_parser p = (streaming_parser){
        .input = file, .length = sb.st_size, .current = file[0]};

    print_document(&p);
}
