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
            auto& index = partial_indices_[current_index];

            // paths will be relative to folder where index was built
            auto path = directory / fs::relative(entry.path(), directory);

            boost::asio::post(builder_pool_, [&index, path, file_id]() { index.addFile(path, file_id); });

            file_id++;
            current_index = (current_index + 1) % partial_indices_.size();
        }
    }

    builder_pool_.join();

    size_t total_files = 0;
    for (auto& index : partial_indices_) {
        total_files += index.getTotalFiles();
    }

    result_ = Index();
    result_.reserve(total_files);
    for (auto& index : partial_indices_) {
        result_.mergeIndex(index);
    }
    
    partial_indices_.clear();
}

const Index& IndexBuilder::getIndex() 
{
    return result_;
}
