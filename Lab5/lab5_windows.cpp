#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


typedef struct {
    int thread_num;
    DWORD_PTR cpu_core_mask; 
    int priority;   

    long start;
    long end;
    int* items;
    long long result_count;
    int result_max_zeros_element;
    long long duration;
} ThreadData;

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

void setup_thread(int thread_num, DWORD_PTR cpu_core_mask, int priority) {
    HANDLE hThread = GetCurrentThread();

    if (cpu_core_mask > 0) {
        printf("Потік %d: Встановлено маску афінності %llu\n", thread_num, cpu_core_mask);
        if (SetThreadAffinityMask(hThread, cpu_core_mask) == 0) {
            printf("Помилка встановлення афінності для потоку %d\n", thread_num);
        }
    }

    if (priority != -255) {
        printf("Потік %d: Встановлено пріоритет %d\n", thread_num, priority);
        if (SetThreadPriority(hThread, priority) == 0) {
            printf("Помилка встановлення пріоритету для потоку %d\n", thread_num);
        }
    }
}

DWORD WINAPI thread_func_cpu(LPVOID lpParam) {
    ThreadData* data = (ThreadData*)lpParam;
    setup_thread(data->thread_num, data->cpu_core_mask, data->priority);

    long long count_matches = 0;
    int target_divisors = 4;

    long long start_time = GetTickCount64();
    for (long i = data->start; i <= data->end; i++) {
        if (count_divisors(i) == target_divisors) {
            count_matches++;
        }
    }
    long long end_time = GetTickCount64();
    
    data->result_count = count_matches;
    data->duration = end_time - start_time;
    return 0;
}

DWORD WINAPI thread_func_memory(LPVOID lpParam) {
    ThreadData* data = (ThreadData*)lpParam;
    setup_thread(data->thread_num, data->cpu_core_mask, data->priority);

    int max_zeros = -1;
    int best_element = 0;
    int* items = data->items;

    long long start_time = GetTickCount64();
    for (long i = data->start; i <= data->end; i++) {
        int zeros = count_zeros(items[i]);
        if (zeros > max_zeros) {
            max_zeros = zeros;
            best_element = items[i];
        }
    }
    long long end_time = GetTickCount64();
    
    data->result_max_zeros_element = best_element;
    data->duration = end_time - start_time;
    return 0;
}

int main() {
    
    SetConsoleOutputCP(CP_UTF8);

    printf("=== Тестування задачі, що навантажує процесор (CPU-bound) ===\n");
    long start_cpu = 1;
    long end_cpu = 3000000;
    long mid_cpu = (start_cpu + end_cpu) / 2;


    ThreadData t1_cpu = { 1, 1, THREAD_PRIORITY_HIGHEST, start_cpu, mid_cpu, NULL, 0, 0, 0 };
    ThreadData t2_cpu = { 2, 2, THREAD_PRIORITY_LOWEST, mid_cpu + 1, end_cpu, NULL, 0, 0, 0 };

    HANDLE hThreads[2];
    hThreads[0] = CreateThread(NULL, 0, thread_func_cpu, &t1_cpu, 0, NULL);
    hThreads[1] = CreateThread(NULL, 0, thread_func_cpu, &t2_cpu, 0, NULL);
    WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);
    CloseHandle(hThreads[0]);
    CloseHandle(hThreads[1]);

    printf("Час виконання CPU-потоку 1: %I64d мс\n", t1_cpu.duration);
    printf("Час виконання CPU-потоку 2: %I64d мс\n", t2_cpu.duration);

    printf("\n=== Тестування задачі, що навантажує пам'ять (Memory-bound) ===\n");
    long start_mem = 0;
    long end_mem = 250000000;
    int* items = (int*)malloc(end_mem * sizeof(int));
    for(long i = 0; i < end_mem; i++) { items[i] = rand(); }
    long mid_mem = end_mem / 2;

    ThreadData t1_mem = { 1, 1, THREAD_PRIORITY_NORMAL, start_mem, mid_mem - 1, items, 0, 0, 0 };
    ThreadData t2_mem = { 2, 1, THREAD_PRIORITY_NORMAL, mid_mem, end_mem - 1, items, 0, 0, 0 };

    hThreads[0] = CreateThread(NULL, 0, thread_func_memory, &t1_mem, 0, NULL);
    hThreads[1] = CreateThread(NULL, 0, thread_func_memory, &t2_mem, 0, NULL);
    WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);
    CloseHandle(hThreads[0]);
    CloseHandle(hThreads[1]);

    printf("Час виконання Memory-потоку 1: %I64d мс\n", t1_mem.duration);
    printf("Час виконання Memory-потоку 2: %I64d мс\n", t2_mem.duration);

    free(items);
    return 0;
}