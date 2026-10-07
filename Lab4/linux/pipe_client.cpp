#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 1024
#define FIFO_P2C "p2c_fifo"
#define FIFO_C2P "c2p_fifo"

int main(void) {
    int fd_write = open(FIFO_P2C, O_WRONLY);
    if (fd_write == -1) { perror("open p2c"); exit(1); }
    int fd_read = open(FIFO_C2P, O_RDONLY);
    if (fd_read == -1) { perror("open c2p"); close(fd_write); exit(1); }

    char input[BUF_SIZE];
    while (true) {
        printf("Enter a string ('exit' to quit): ");
        fflush(stdout);
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0;

        write(fd_write, input, strlen(input)+1);
        if (strcmp(input, "exit") == 0) break;

        char output[BUF_SIZE];
        ssize_t r = read(fd_read, output, sizeof(output)-1);
        if (r > 0) {
            output[r] = '\0';
            printf("Response from worker: %s\n", output);
        }
    }

    close(fd_write);
    close(fd_read);
    return 0;
}
