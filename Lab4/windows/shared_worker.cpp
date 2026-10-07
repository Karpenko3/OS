#include <windows.h>
#include <iostream>
#include <string>
#include <cctype>

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
    HANDLE hMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, sharedName);
    auto* data = (SharedData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));

    while (true) {
        while (!data->ready) Sleep(10);
        if (data->exit_flag) break;

        data->ready = false; 

        string str(data->src);
        string out_str = "";
        for (char c : str) {
            if (isalpha(static_cast<unsigned char>(c))) {
                out_str += ' ';
            } else {
                out_str += c;
            }
        }
        
        strncpy(data->dest, out_str.c_str(), sizeof(data->dest));
        data->done = true;
    }

    UnmapViewOfFile(data);
    CloseHandle(hMapFile);
    return 0;
}
