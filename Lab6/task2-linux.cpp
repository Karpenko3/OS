#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <atomic>
#include <string>

long long counter = 0;                       
std::atomic<long long> atomic_counter(0);    

pthread_mutex_t lock;

typedef struct {
    int id;
    int num_threads;
    long long max_num;
    int target_factors;
} targs_t;

int count_factors(long long n) {
    if (n < 2) return 0;
    int factors = 0;
    for (long long d = 2; d * d <= n; d++) {
        while (n % d == 0) {
            factors++;
            n /= d;
        }
    }
    if (n > 1) {
        factors++;
    }
    return factors;
}

void* ThreadFuncNoSync(void* lpParam) {
    targs_t* t = (targs_t*)lpParam;

    for (long long n = 2 + t->id; n <= t->max_num; n += t->num_threads) {
        if (count_factors(n) == t->target_factors) {
            counter++;
        }
    }

    return 0;
}

void* ThreadFuncAtomic(void* lpParam) {
    targs_t* t = (targs_t*)lpParam;

    for (long long n = 2 + t->id; n <= t->max_num; n += t->num_threads) {
        if (count_factors(n) == t->target_factors) {
            atomic_counter++;
        }
    }

    return 0;
}

void* ThreadFuncMutex(void* lpParam) {
    targs_t* t = (targs_t*)lpParam;

    for (long long n = 2 + t->id; n <= t->max_num; n += t->num_threads) {
        if (count_factors(n) == t->target_factors) {
            pthread_mutex_lock(&lock);
            counter++;
            pthread_mutex_unlock(&lock);
        }
    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <num_threads> <max_num> [mode] [target_factors]\n", argv[0]);
        fprintf(stderr, "  mode = no_sync - No sync\n  mode = atomic - atomic variables\n");
        fprintf(stderr, "  mode = mutex - use mutex for synchronization\n");
        return 1;
    }

    int num_threads = atoi(argv[1]);
    long long max_num = atoll(argv[2]);
    std::string sync_mode = "no_sync";
    if (argc >= 4) sync_mode = argv[3];
    int target_factors = 3;
    if (argc >= 5) target_factors = atoi(argv[4]);

    void* (*thread_func)(void*) = NULL;
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

    pthread_t* threads = new pthread_t[num_threads];
    targs_t* args = new targs_t[num_threads];

    pthread_mutex_init(&lock, NULL);

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    counter = 0;
    atomic_counter = 0;

    for (int i = 0; i < num_threads; ++i) {
        args[i].id = i;
        args[i].num_threads = num_threads;
        args[i].max_num = max_num;
        args[i].target_factors = target_factors;
        if (pthread_create(&threads[i], NULL, thread_func, &args[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long elapsed = 1000 * (end_time.tv_sec - start_time.tv_sec) +
                   (end_time.tv_nsec - start_time.tv_nsec) / 1000000;

    long long expected = 0;
    for (long long n = 2; n <= max_num; ++n) {
        if (count_factors(n) == target_factors) {
            expected++;
        }
    }

    printf("Task: Count numbers in [2, %lld] with exactly %d prime factors\n", max_num, target_factors);
    printf("Synchronization mode: %s\n", sync_mode.c_str());
    printf("Expected counter = %lld\n", expected);
    if (sync_mode == "atomic") {
        printf("Actual   counter = %lld\n", atomic_counter.load());
        printf("Atomic lock-free = %d\n", atomic_counter.is_lock_free());
    }
    else {
        printf("Actual   counter = %lld\n", counter);
    }
    printf("Time elapsed     = %ld ms\n", elapsed);

    pthread_mutex_destroy(&lock);
    delete[] threads;
    delete[] args;
    return 0;
}