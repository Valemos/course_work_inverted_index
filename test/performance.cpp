
#include <iostream>
#include <chrono>
#include <functional>
#include <filesystem>

#include "index/Index.h"
#include "index/IndexBuilder.h"
#include "misc/user_input.h"


void measureAverageTime(std::function<void()> task, int repeat_number) {
    double total_time {0};

    for (int i = 0; i < repeat_number; i++) {
        
        auto start = std::chrono::high_resolution_clock::now();
        task();
        auto end = std::chrono::high_resolution_clock::now();

        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        total_time += time.count();
        std::cout << "\trun " << i + 1 << " time: " << time.count() << "ms" << std::endl;
    }

    std::cout << "average time: " << total_time / repeat_number << "ms" << std::endl;
}


int main(int argc, const char** argv) {
    const int repeats_for_average = 5;
    std::filesystem::path index_dir {user_input::promptExistingDirectory()};

    auto index_task = [index_dir](int threads_number){
        IndexBuilder builder;
        builder.indexDirectory(index_dir);
    };

    auto single_threaded_task = [index_dir](){
        Index index;
        for (auto& entry : fs::recursive_directory_iterator(fs::absolute(index_dir))) {
            if (entry.is_regular_file()) {
                index.addFile(entry.path());
            }
        }
    };

    std::cout << "single threaded task: " << std::endl; 
    measureAverageTime(single_threaded_task, repeats_for_average);

    for (auto threads_number : std::vector<int> {1, 2, 4, 8, 16, 32}) {
        std::cout << std::endl << threads_number << " threads:" << std::endl;
        measureAverageTime(std::bind(index_task, threads_number), repeats_for_average);
    }

    std::cin.get();

    return 0;
}