
#include <iostream>
#include <filesystem>

#include "index/Index.h"


fs::path getPath(std::string why) {
    fs::path path;
    std::cout << "enter " << why << ": " << std::endl;
    std::cin >> path;
    return path;
}

fs::path getDirectory() {
    while (true) {
        fs::path path = fs::absolute(getPath("index directory"));
        if (fs::exists(path)) {
            return path;
        }
        std::cout << "path not exists!" << std::endl;
    }
}

int main(int, char**) {
    Index index;
    index.createFromDirectory(getDirectory());
    index.save(getPath("save path"));
    return 0;
}
