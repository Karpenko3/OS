#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// Прапорець для циклу та лічильник натискань
volatile sig_atomic_t keep_running = 1;
volatile sig_atomic_t ctrl_c_count = 0;

void handle_signal(int sig) {
    if (sig == SIGINT) {
        ctrl_c_count++; // Збільшуємо лічильник при кожному Ctrl-C
        printf("\nCaught SIGINT (Ctrl-C). Count: %d/5\n", ctrl_c_count);
        // Якщо натиснули 5 разів — дозволяємо програмі зупинитися
        if (ctrl_c_count >= 5) {
            printf("Limit reached. Exiting gracefully...\n");
            keep_running = 0;
        }
    } else if (sig == SIGTERM) {
        printf("\nCaught SIGTERM (kill). Exiting gracefully...\n");
        keep_running = 0; 
    }
}

int main(void) {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(1);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction SIGTERM");
        exit(1);
    }

    printf("Process running with PID %d\n", getpid());
    printf("Press Ctrl-C or send 'kill %d' to test.\n", getpid());

    while (keep_running) {
        printf("Working...\n");
        sleep(2);
    }

    printf("Cleanup done. Bye!\n");
    return 0;
}