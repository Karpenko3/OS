#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include <string>

long long counter = 0;    // shared counter
std::atomic<long long> atomic_counter(0);    //atomic counter

pthread_mutex_t lock;

typedef struct {
    int id;
    long long increments;
    int use_mutex;
} targs_t;

void* ThreadFuncNoSync(void* lpParam) {
    targs_t* t = (targs_t*)lpParam;

    for (int i = 0; i < t->increments; i++) {
        counter++;
    }

    return 0;
}

void* ThreadFuncAtomic(void* lpParam) {
    targs_t* t = (targs_t*)lpParam;

    for (int i = 0; i < t->increments; i++) {
        atomic_counter++;
    }

    return 0;
}

void* ThreadFuncMutex(void* lpParam) {
    targs_t* t = (targs_t*)lpParam; 

    for (int i = 0; i < t->increments; i++) {
        pthread_mutex_lock(&lock);

        counter++;

        pthread_mutex_unlock(&lock);
    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <num_threads> <inc_per_thread> [use_mutex]\n", argv[0]);
        fprintf(stderr, "  mode = no_sync - No sync\n  mode = atomic - atomic variables\n");
        fprintf(stderr, "  mode = mutex - use mutex for synchronization\n");
        return 1;
    }
    int num_threads = atoi(argv[1]);
    long long inc_per_thread = atoll(argv[2]);
    std::string sync_mode = "no_sync";
    if (argc >= 4) sync_mode = argv[3];
    void *(*thread_func) (void *)  = NULL;
    if (sync_mode == "no_sync") {
        thread_func = ThreadFuncNoSync;
    }
    else if (sync_mode == "atomic") {
        thread_func = ThreadFuncAtomic;
    }
    else if (sync_mode == "mutex") {
        thread_func = ThreadFuncMutex;
    }
    else {
        fprintf(stderr, "Mode %s is not supported\n", sync_mode.c_str());
        return 1;
    }

    pthread_t *threads = new pthread_t[num_threads];
    targs_t *args = new targs_t[num_threads];

    pthread_mutex_init(&lock, NULL);

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    counter = 0;
    for (int i = 0; i < num_threads; ++i) {
        args[i].id = i;
        args[i].increments = inc_per_thread;
        if (pthread_create(&threads[i], NULL, thread_func, &args[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long elapsed = 1000*(end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1000000;

    long long expected = (long long)num_threads * inc_per_thread;
    printf("Synchronization mode: %s\n", sync_mode.c_str());
    printf("Expected counter = %lld\n", expected);
    if (sync_mode == "atomic") {
        printf("Actual   counter = %lld\n", atomic_counter.load());
        printf("Atomic lock-free = %d\n", atomic_counter.is_lock_free());

    }
    else {
        printf("Actual   counter = %lld\n", counter);
    }

    printf("Time elapsed     = %ld\n", elapsed);

    pthread_mutex_destroy(&lock);
    free(threads);
    free(args);
    return 0;
}
