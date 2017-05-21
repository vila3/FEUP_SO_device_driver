#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <device path>\n", argv[0]);
    }
    int f = open(argv[1], O_RDWR);
    write(f, "Hello my friend!", 17);
    char buffer='a';
    int chk;
    while (1) {
        chk = read(f, &buffer, 1);
        if (chk < 0) continue;
        printf("%c\n", buffer);
        break;
    }
    printf("\n");
    close(f);
    return 0;
}
