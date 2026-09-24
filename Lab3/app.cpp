#include <iostream>
#include <string>
#include <cmath>

int main(int argc, char* argv[]) {
    
    if (argc != 3) {
        return 255;
    }
    std::string str1 = argv[1];
    std::string str2 = argv[2];
    
    int diff = std::abs((int)str1.length() - (int)str2.length());
    return diff;
}
