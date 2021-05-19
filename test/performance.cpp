
#include <iostream>
#include <chrono>
#include <functional>
#include <filesystem>

#include "index/Index.h"
#include "index/IndexBuilder.h"


void measureAverageTime(std::function<void()> task, int repeat_number = 5) {
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
    std::filesystem::path index_dir {"D:\\coding\\c_c++\\concurrent_index_course_work\\datasets\\data\\aclImdb\\test\\neg"};

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
    measureAverageTime(single_threaded_task, 10);

    for (auto threads_number : std::vector<int> {1, 2, 4, 8}) {
        std::cout << std::endl << threads_number << " threads:" << std::endl;
        measureAverageTime(std::bind(index_task, threads_number), 10);
    }

    return 0;
}