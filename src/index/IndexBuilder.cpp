#include "IndexBuilder.h"

#include <boost/log/trivial.hpp>

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

            BOOST_LOG_TRIVIAL(trace) << "posted for file " << entry.path().stem() 
                                    << " id: " << file_id 
                                    << " index " << current_index;
            boost::asio::post(builder_pool_, [&index, path, file_id]() { index.addFile(path, file_id); });

            file_id++;
            current_index = (current_index + 1) % partial_indices_.size();
        }
    }

    BOOST_LOG_TRIVIAL(trace) << "awaiting for tasks to finish";
    builder_pool_.join();
    BOOST_LOG_TRIVIAL(trace) << "partial build finished";

    result_ = Index();
    for (auto& index : partial_indices_) {
        result_.mergeIndex(index);
    }
    BOOST_LOG_TRIVIAL(trace) << "indices merged";

    partial_indices_.clear();
}

const Index& IndexBuilder::getIndex() 
{
    return result_;
}
