#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

struct SharedData {
    char src[256];
    char dest[256];
    bool ready;
    bool done;
    bool exit_flag;
};

int main() {
    const wchar_t* sharedName = L"MySharedMemory";
    const wchar_t* workerExe = L"shared_worker.exe"; 

    HANDLE hMapFile = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedData), sharedName);
    auto* data = (SharedData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));
    data->ready = false; data->done = false; data->exit_flag = false;

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    CreateProcessW(workerExe, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    while (true) {
        string input;
        cout << "Enter string ('exit' to quit): ";
        getline(cin, input);

        if (input == "exit") {
            data->exit_flag = true; data->ready = true;
            break;
        }

        strncpy(data->src, input.c_str(), sizeof(data->src));
        data->done = false; data->ready = true;

        while (!data->done) Sleep(10);
        cout << "Result from worker: " << data->dest << "\n\n";
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    UnmapViewOfFile(data); CloseHandle(hMapFile);
    return 0;
}
