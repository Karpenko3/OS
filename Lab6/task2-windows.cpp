#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <atomic>
#include <string>

// Shared global data
long long counter = 0;
std::atomic<long long> atomic_counter(0);

typedef struct {
    int id;
    int num_threads;
    long long max_num;
    int target_factors;
} targs_t;

// Synchronization primitives
HANDLE hMutex;
CRITICAL_SECTION cs;

// CPU-bound задача: підрахунок кількості простих множників числа n
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

DWORD WINAPI ThreadFuncNoSync(LPVOID lpParam) {
    targs_t* t = (targs_t*)lpParam;

    for (long long n = 2 + t->id; n <= t->max_num; n += t->num_threads) {
        if (count_factors(n) == t->target_factors) {
            counter++;
        }
    }

    return 0;
}

DWORD WINAPI ThreadFuncAtomic(LPVOID lpParam) {
    targs_t* t = (targs_t*)lpParam;

    for (long long n = 2 + t->id; n <= t->max_num; n += t->num_threads) {
        if (count_factors(n) == t->target_factors) {
            atomic_counter++;
        }
    }

    return 0;
}

DWORD WINAPI ThreadFuncMutex(LPVOID lpParam) {
    targs_t* t = (targs_t*)lpParam;

    for (long long n = 2 + t->id; n <= t->max_num; n += t->num_threads) {
        if (count_factors(n) == t->target_factors) {
            WaitForSingleObject(hMutex, INFINITE);
            counter++;
            ReleaseMutex(hMutex);
        }
    }

    return 0;
}

DWORD WINAPI ThreadFuncCS(LPVOID lpParam) {
    targs_t* t = (targs_t*)lpParam;

    for (long long n = 2 + t->id; n <= t->max_num; n += t->num_threads) {
        if (count_factors(n) == t->target_factors) {
            EnterCriticalSection(&cs);
            counter++;
            LeaveCriticalSection(&cs);
        }
    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <num_threads> <max_num> [mode] [target_factors]\n", argv[0]);
        fprintf(stderr, "  mode = no_sync - No sync\n  mode = atomic - atomic variables\n");
        fprintf(stderr, "  mode = mutex - use mutex for synchronization\n  mode = cs - use critical sections for synchronization\n");
        return 1;
    }

    int num_threads = atoi(argv[1]);
    long long max_num = atoll(argv[2]);
    std::string sync_mode = "no_sync";
    if (argc >= 4) sync_mode = argv[3];
    int target_factors = 3;
    if (argc >= 5) target_factors = atoi(argv[4]);

    LPTHREAD_START_ROUTINE thread_func = NULL;
    if (sync_mode == "no_sync") {
        thread_func = ThreadFuncNoSync;
    }
    else if (sync_mode == "atomic") {
        thread_func = ThreadFuncAtomic;
    }
    else if (sync_mode == "mutex") {
        thread_func = ThreadFuncMutex;
    }
    else if (sync_mode == "cs") {
        thread_func = ThreadFuncCS;
    }
    else {
        fprintf(stderr, "Mode %s is not supported\n", sync_mode.c_str());
        return 1;
    }

    HANDLE* threads = new HANDLE[num_threads];
    DWORD* threadIds = new DWORD[num_threads];
    targs_t* threadArgs = new targs_t[num_threads];

    hMutex = CreateMutex(NULL, FALSE, NULL);
    if (hMutex == NULL) {
        fprintf(stderr, "CreateMutex failed (%lu)\n", GetLastError());
        return 1;
    }
    InitializeCriticalSection(&cs);

    counter = 0;
    atomic_counter = 0;

    long long start = GetTickCount64();

    for (int i = 0; i < num_threads; i++) {
        threadArgs[i].id = i;
        threadArgs[i].num_threads = num_threads;
        threadArgs[i].max_num = max_num;
        threadArgs[i].target_factors = target_factors;

        threads[i] = CreateThread(NULL, 0, thread_func, (LPVOID)(threadArgs + i), 0, &threadIds[i]);
        if (threads[i] == NULL) {
            printf("CreateThread failed (%lu)\n", GetLastError());
            return 1;
        }
    }

    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);
    long long stop = GetTickCount64();

    for (int i = 0; i < num_threads; i++) {
        CloseHandle(threads[i]);
    }
    CloseHandle(hMutex);
    DeleteCriticalSection(&cs);

    long long elapsed = stop - start;

  
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
    printf("Time elapsed     = %lld ms\n", elapsed);

    delete[] threads;
    delete[] threadIds;
    delete[] threadArgs;
    return 0;
}