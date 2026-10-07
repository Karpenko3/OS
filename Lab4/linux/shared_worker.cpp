#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

struct SharedData {
    char src[256];
    char dest[256];
    bool ready;
    bool done;
    bool exit_flag;
};

void replace_letters(char *str, char *dest) {
    size_t i;
    for (i = 0; i < strlen(str); i++) {
        if (isalpha((unsigned char)str[i])) {
            dest[i] = ' ';
        } else {
            dest[i] = str[i];
        }
    }
    dest[i] = '\0';
}

int main() {
    const char* shmName = "/my_shared_memory";
    int fd = shm_open(shmName, O_RDWR, 0666);
    auto* data = (SharedData*)mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    while (true) {
        while (!data->ready) usleep(10000);
        
        if (data->exit_flag) break;

      
        data->ready = false;

        printf("Worker received: %s\n", data->src);
        replace_letters(data->src, data->dest);
        
        data->done = true;
    }

    munmap(data, sizeof(SharedData));
    close(fd);
    return 0;
}
