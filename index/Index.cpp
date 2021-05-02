#include "Index.h"

#include <filesystem>
#include <fstream>
#include <boost/log/trivial.hpp>

namespace fs = std::filesystem;



Index::Index(size_t total_files) 
{
    document_paths_.reserve(total_files);
}

void Index::createFromDirectory(std::string directory_path) 
{
    for (auto& entry : fs::recursive_directory_iterator(directory_path)){
        if (entry.is_regular_file()) {
            addFileIndex(entry.path().string());
        }
    }
}

void Index::addFileIndex(std::string path) 
{
    BOOST_LOG_TRIVIAL(info) << path;

    // path_id is index of document_paths_ vector
    int path_id = document_paths_.size();
    document_paths_.push_back(path);

    Index::TermPosition current_position;

    std::ifstream fin(path);
    if (!fin.bad()){

        // todo iterate all words and add them to index

    } else {
        BOOST_LOG_TRIVIAL(error) << "cannot read file " << path;
    }
}
