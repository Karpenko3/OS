#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

int main() {
    const wchar_t* pipeName = L"\\\\.\\pipe\\mypipe";
    HANDLE hPipe = CreateFileW(pipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    while (true) {
        string input;
        cout << "Input string ('exit' to quit): ";
        getline(cin, input);
        
        DWORD bytesWritten, bytesRead;
        WriteFile(hPipe, input.c_str(), input.size(), &bytesWritten, NULL);
        if (input == "exit") break;

        char buffer[1024];
        ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        buffer[bytesRead] = '\0';
        cout << "Response: " << buffer << "\n\n";
    }
    CloseHandle(hPipe);
    return 0;
}
