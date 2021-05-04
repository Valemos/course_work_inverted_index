
#include <iostream>
#include <filesystem>

#include "misc/user_input.h"
#include "index/Index.h"


int main(int, char**) {
    Index index;
    index.createFromDirectory(user_input::promptExistingDirectory());
    std::cout << "enter save path: " << std::endl;
    index.save(user_input::promptOnce());
    return 0;
}
