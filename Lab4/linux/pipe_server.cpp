#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 1024
#define FIFO_P2C "p2c_fifo"
#define FIFO_C2P "c2p_fifo"

int main(void) {
    if (mkfifo(FIFO_P2C, 0666) == -1 && errno != EEXIST) { perror("mkfifo p2c"); exit(1); }
    if (mkfifo(FIFO_C2P, 0666) == -1 && errno != EEXIST) { perror("mkfifo c2p"); exit(1); }

    int fd_read = open(FIFO_P2C, O_RDONLY);
    if (fd_read == -1) { perror("open p2c"); exit(1); }
    int fd_write = open(FIFO_C2P, O_WRONLY);
    if (fd_write == -1) { perror("open c2p"); close(fd_read); exit(1); }

    while (true) {
        char buffer[BUF_SIZE];
        ssize_t r = read(fd_read, buffer, sizeof(buffer)-1);
        if (r <= 0) break;
        buffer[r] = '\0';
        
        if (strncmp(buffer, "exit", 4) == 0) break;
        printf("Worker received: %s\n", buffer);

        for (size_t i = 0; i < strlen(buffer); i++) {
            if (isalpha((unsigned char)buffer[i])) buffer[i] = ' ';
        }

        if (write(fd_write, buffer, strlen(buffer)+1) == -1) perror("write");
    }

    close(fd_read);
    close(fd_write);
    return 0;
}
