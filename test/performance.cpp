
#include <gtest/gtest.h>
#include <iostream>
#include <chrono>
#include <functional>

#include <filesystem>

#include "index/Index.h"
#include "index/IndexBuilder.h"
#include "misc/user_input.h"


void measureAverageTime(std::function<void()> &task, int repeat_number) {
    double total_time {0};

    for (int i = 0; i < repeat_number; i++) {
        try {
            auto start = std::chrono::high_resolution_clock::now();
            task();
            auto end = std::chrono::high_resolution_clock::now();

            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            total_time += (double)time.count();
            std::cout << "\trun " << i + 1 << "> time: " << time.count() << "ms" << std::endl;   
        } catch (std::exception& err) {
            std::cout << "\trun " << i + 1 << "> error: "<< err.what() << std::endl;
        }
    }

    std::cout << "average time: " << total_time / repeat_number << "ms" << std::endl;
}


TEST(TestPerformance, Display) {
    const int repeats_for_average = 10;
    std::filesystem::path index_dir {user_input::promptExistingDirectory()};

    std::function<void()> single_threaded_task = [index_dir](){
        Index index;
        for (auto& entry : fs::recursive_directory_iterator(fs::absolute(index_dir))) {
            if (entry.is_regular_file()) {
                index.addFile(entry.path());
            }
        }
    };

    std::cout << "single threaded task: " << std::endl; 
    measureAverageTime(single_threaded_task, repeats_for_average);

    for (auto threads_number : std::vector<int> {1, 2, 4, 8, 16}) {
        std::cout << std::endl << threads_number << " pool threads:" << std::endl;

        std::function<void()> index_task = [&index_dir, threads_number](){
            IndexBuilder builder {threads_number};
            builder.indexDirectory(index_dir);
        };

        measureAverageTime(index_task, repeats_for_average);
    }

    std::cin.get();
}