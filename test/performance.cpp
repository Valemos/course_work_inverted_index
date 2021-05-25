
#include <iostream>
#include <chrono>
#include <functional>
#include <filesystem>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "index/Index.h"
#include "index/IndexBuilder.h"
#include "misc/user_input.h"


void measureAverageTime(std::function<void()> task, int repeat_number) {
    double total_time {0};

    for (int i = 0; i < repeat_number; i++) {
        try {
            auto start = std::chrono::high_resolution_clock::now();
            task();
            auto end = std::chrono::high_resolution_clock::now();

            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            total_time += time.count();
            std::cout << "\trun " << i + 1 << "> time: " << time.count() << "ms" << std::endl;   
        } catch (std::exception& err) {
            std::cout << "\trun " << i + 1 << "> error: "<< err.what() << std::endl;
        }
    }

    std::cout << "average time: " << total_time / repeat_number << "ms" << std::endl;
}


int main(int argc, const char** argv) {
    boost::log::core::get()->set_filter (boost::log::trivial::severity >= boost::log::trivial::trace);

    const int repeats_for_average = 10;
    std::filesystem::path index_dir {"D:/coding/c_c++/concurrent_index_course_work/datasets/data/aclImdb/test/neg"};
    // std::filesystem::path index_dir {user_input::promptExistingDirectory()};

    auto single_threaded_task = [index_dir](){
        Index index;
        for (auto& entry : fs::recursive_directory_iterator(fs::absolute(index_dir))) {
            if (entry.is_regular_file()) {
                index.addFile(entry.path());
            }
        }
    };

    // std::cout << "single threaded task: " << std::endl; 
    // measureAverageTime(single_threaded_task, repeats_for_average);
    IndexBuilder* builder {nullptr};
    for (auto threads_number : std::vector<int> {2, 4, 8}) {
        std::cout << std::endl << threads_number << " threads:" << std::endl;

        auto index_task = [&index_dir, threads_number, &builder](){
            builder = new IndexBuilder(threads_number);
            builder->indexDirectory(index_dir);
            delete builder;
        };

        measureAverageTime(index_task, repeats_for_average);
    }

    std::cin.get();

    return 0;
}