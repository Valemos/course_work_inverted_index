#include "IndexBuilder.h"

#include <thread>
#include <functional>
#include <boost/asio/post.hpp>


// by default takes all available threads except main thread
IndexBuilder::IndexBuilder() :
    threads_number_(std::thread::hardware_concurrency() - 1),
    builder_pool_(std::thread::hardware_concurrency() - 1)
{
}

IndexBuilder::IndexBuilder(int threads_number) :
    threads_number_(threads_number),
    builder_pool_(threads_number)
{
}

void IndexBuilder::indexDirectory(fs::path directory) 
{
    // create index objects from different files and merge them into one
    partial_indices_.clear();
    partial_indices_.resize(threads_number_);
    
    int file_id = 0;
    int current_index = 0;
    for (auto& entry : fs::recursive_directory_iterator(fs::absolute(directory))) {
        if (entry.is_regular_file()) {
            auto* index_ptr = &partial_indices_[current_index];

            // index will be located in the same root as target directory
            auto path = directory / fs::relative(entry.path(), directory);

            auto task = [index_ptr, path, file_id]() { index_ptr->addFile(path, file_id); };
            boost::asio::post(builder_pool_, task);

            file_id++;
            current_index = (current_index + 1) % partial_indices_.size();
        }
    }

    builder_pool_.join();

    size_t total_files = 0;
    for (auto& index : partial_indices_) {
        total_files += index.getTotalFiles();
    }

    result_.reserve(total_files);
    for (auto& index : partial_indices_) {
        result_.mergeIndex(index);
    }
}

const Index& IndexBuilder::getIndex() 
{
    return result_;
}
