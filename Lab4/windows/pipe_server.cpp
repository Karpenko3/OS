#include <windows.h>
#include <iostream>
#include <string>
#include <cctype>

using namespace std;

int main() {
    const wchar_t* pipeName = L"\\\\.\\pipe\\mypipe";
    HANDLE hPipe = CreateNamedPipeW(pipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 1024, 1024, 0, NULL);
    ConnectNamedPipe(hPipe, NULL);

    char buffer[1024];
    DWORD bytesRead, bytesWritten;

    while (true) {
        BOOL result = ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        if (!result || bytesRead == 0) break;
        buffer[bytesRead] = '\0';
        string input(buffer);
        
        string output = "";
        for (char c : input) {
            if (isalpha(static_cast<unsigned char>(c))) {
                output += ' ';
            } else {
                output += c;
            }
        }

        WriteFile(hPipe, output.c_str(), output.size(), &bytesWritten, NULL);
    }
    CloseHandle(hPipe);
    return 0;
}
