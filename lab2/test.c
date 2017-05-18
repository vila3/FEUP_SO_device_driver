#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <device path>\n", argv[0]);
    }
    int f = open(argv[1], O_WRONLY|O_RDONLY);
    write(f, "Hello\n", 6);
    close(f);
    return 0;
}
