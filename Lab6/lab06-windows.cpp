#include <windows.h>
#include <stdio.h>
#include <atomic> 
#include <string>

// Shared data
long long counter = 0;
std::atomic<long long> atomic_counter(0);

typedef struct {
    int id;
    long long increments;
} targs_t;

// Mutex handle
HANDLE hMutex;
CRITICAL_SECTION cs;

DWORD WINAPI ThreadFuncNoSync(LPVOID lpParam) {
    targs_t* t = (targs_t*)lpParam;

    for (int i = 0; i < t->increments; i++) {
        counter++;
    }

    return 0;
}

DWORD WINAPI ThreadFuncAtomic(LPVOID lpParam) {
    targs_t* t = (targs_t*)lpParam;

    for (int i = 0; i < t->increments; i++) {
        atomic_counter++;
    }

    return 0;
}

DWORD WINAPI ThreadFuncMutex(LPVOID lpParam) {
    targs_t* t = (targs_t*)lpParam; 

    for (int i = 0; i < t->increments; i++) {
        WaitForSingleObject(hMutex, INFINITE);

        counter++;

        ReleaseMutex(hMutex);
    }

    return 0;
}

DWORD WINAPI ThreadFuncCS(LPVOID lpParam) {
    targs_t* t = (targs_t*)lpParam;

    for (int i = 0; i < t->increments; i++) {
        EnterCriticalSection(&cs);

        counter++;

        LeaveCriticalSection(&cs);
    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <num_threads> <inc_per_thread> [mode]\n", argv[0]);
        fprintf(stderr, "  mode = no_sync - No sync\n  mode = atomic - atomic variables\n");
        fprintf(stderr, "  mode = mutex - use mutex for synchronization\n  mode = cs - use critical sections for synchronization\n");
        return 1;
    }
    int num_threads = atoi(argv[1]);
    long long inc_per_thread = atoll(argv[2]);

    std::string sync_mode = "no_sync";
    if (argc >= 4) sync_mode = argv[3];
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

    HANDLE *threads = new HANDLE[num_threads];
    DWORD *threadIds = new DWORD[num_threads];
    targs_t* threadArgs = new targs_t[num_threads];

    // Create a mutex (initially not owned)
    hMutex = CreateMutex(NULL, FALSE, NULL);
    if (hMutex == NULL) {
        fprintf(stderr, "CreateMutex failed (%lu)\n", GetLastError());
        return 1;
    }
    InitializeCriticalSection(&cs);

    long long start = GetTickCount64();
    
    // Create threads
    for (int i = 0; i < num_threads; i++) {
        threadArgs[i].id = i;
        threadArgs[i].increments = inc_per_thread;
        
        threads[i] = CreateThread(NULL, 0, thread_func, (LPVOID)(threadArgs + i), 0, &threadIds[i]);
        if (threads[i] == NULL) {
            printf("CreateThread failed (%lu)\n", GetLastError());
            return 1;
        }
    }

    // Wait for all threads to finish
    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);
    long long stop = GetTickCount64();

    // Cleanup
    for (int i = 0; i < num_threads; i++) {
        CloseHandle(threads[i]);
    }
    CloseHandle(hMutex);

    // Print result
    long long elapsed = stop - start;
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

    return 0;
}
