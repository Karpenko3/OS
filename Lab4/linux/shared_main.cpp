#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>

struct SharedData {
    char src[256];
    char dest[256];
    bool ready;
    bool done;
    bool exit_flag;
};

int main() {
    const char* shmName = "/my_shared_memory";
    int fd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(SharedData));
    auto* data = (SharedData*)mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    data->ready = false; data->done = false; data->exit_flag = false;

    pid_t pid = fork();
    if (pid == 0) {
        execl("./shared_worker", "shared_worker", NULL);
        return 1;
    }

    while (true) {
        printf("Enter string ('exit' to quit): ");
        if (fgets(data->src, 256, stdin) == NULL) break;
        data->src[strcspn(data->src, "\n")] = 0;

        if (strcmp(data->src, "exit") == 0) {
            data->exit_flag = true; data->ready = true;
            break;
        }

        data->done = false;
        data->ready = true; // Даємо команду робітнику

        while (!data->done) usleep(10000); // Чекаємо на результат
        
        printf("Result: %s\n\n", data->dest);
    }

    wait(NULL);
    munmap(data, sizeof(SharedData));
    shm_unlink(shmName);
    return 0;
}
