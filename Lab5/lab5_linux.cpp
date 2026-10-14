#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include <errno.h>
#include <sys/resource.h>

#define NUM_THREADS 2

typedef struct {
    int thread_num;
    int cpu_core;   
    int priority;   

    long start;
    long end;
    int* items;
    long long result_count; 
    int result_max_zeros_element; 
    long duration;
} ThreadData;

void setup_thread(pthread_t thread, int cpu_core, int priority) {
    if (cpu_core >= 0){
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_core, &cpuset);
        printf("Потік %lu: Встановлено прив'язку до ядра %d\n", pthread_self(), cpu_core);
        if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
            perror("Помилка встановлення афінності (pthread_setaffinity_np)");
        }
    }

    if (priority >= -20 && priority <= 19) {
        printf("Потік %lu: Встановлено пріоритет (nice) на %d\n", pthread_self(), priority);
        pid_t tid = gettid();
        if (setpriority(PRIO_PROCESS, tid, priority) == -1) {
            perror("Помилка встановлення пріоритету (чи запущено програму через sudo?)");
        }
    }
}

int count_divisors(long n) {
    int count = 0;
    for (long i = 1; i * i <= n; i++) {
        if (n % i == 0) {
            count++;
            if (i * i != n) count++;
        }
    }
    return count;
}

int count_zeros(int n) {
    if (n == 0) return 1;
    int count = 0;
    n = abs(n);
    while (n > 0) {
        if (n % 10 == 0) count++;
        n /= 10;
    }
    return count;
}


void *thread_func_cpu(void *lpParam) {
    ThreadData* data = (ThreadData*)lpParam;
    setup_thread(pthread_self(), data->cpu_core, data->priority);

    long long count_matches = 0;
    int target_divisors = 4; 

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (long i = data->start; i <= data->end; i++) {
        if (count_divisors(i) == target_divisors) {
            count_matches++;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long elapsed = 1000*(end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1000000;

    data->result_count = count_matches;
    data->duration = elapsed;
    return 0;
}

void *thread_func_memory(void *lpParam) {
    ThreadData* data = (ThreadData*)lpParam;
    setup_thread(pthread_self(), data->cpu_core, data->priority);

    int max_zeros = -1;
    int best_element = 0;
    int* items = data->items;

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (long i = data->start; i <= data->end; i++) {
        int zeros = count_zeros(items[i]);
        if (zeros > max_zeros) {
            max_zeros = zeros;
            best_element = items[i];
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long elapsed = 1000*(end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1000000;

    data->result_max_zeros_element = best_element;
    data->duration = elapsed;
    return 0;
}

int main(int argc, char *argv[]) {
    printf("=== Тестування задачі, що навантажує процесор (CPU-bound) ===\n");
    long start_cpu = 1;
    long end_cpu = 3000000; 
    long mid_cpu = (start_cpu + end_cpu) / 2;

    ThreadData tdata_cpu[2];
  
    tdata_cpu[0] = { 1, 0, 0, start_cpu, mid_cpu, NULL, 0, 0, 0 };
    tdata_cpu[1] = { 2, 2, 0, mid_cpu + 1, end_cpu, NULL, 0, 0, 0 };

    pthread_t threads[NUM_THREADS];
    for (int i=0; i<2; i++) pthread_create(&threads[i], NULL, thread_func_cpu, &tdata_cpu[i]);
    for (int i=0; i<2; i++) pthread_join(threads[i], NULL);
    for (int i=0; i<2; i++) printf("Час виконання CPU-потоку %d: %ld мс\n", i+1, tdata_cpu[i].duration);


    printf("\n=== Тестування задачі, що навантажує пам'ять (Memory-bound) ===\n");
    long start_mem = 0;
    long end_mem = 250000000; 
    int* items = (int*)malloc(end_mem * sizeof(int));
    if (!items) { printf("Помилка виділення пам'яті!\n"); return 1; }
    
   
    for(long i = 0; i < end_mem; i++) {
        items[i] = rand();
    }
    long mid_mem = end_mem / 2;

    ThreadData tdata_mem[2];
  
    tdata_mem[0] = { 1, 0, 0, start_mem, mid_mem - 1, items, 0, 0, 0 };
    tdata_mem[1] = { 2, 2, 0, mid_mem, end_mem - 1, items, 0, 0, 0 };

    for (int i=0; i<2; i++) pthread_create(&threads[i], NULL, thread_func_memory, &tdata_mem[i]);
    for (int i=0; i<2; i++) pthread_join(threads[i], NULL);
    for (int i=0; i<2; i++) printf("Час виконання Memory-потоку %d: %ld мс\n", i+1, tdata_mem[i].duration);

    free(items);
    return 0;
}