#include "Index.h"

#include <boost/log/trivial.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;


Index::Index(size_t total_files) 
{
    document_paths_.reserve(total_files);
}

void Index::createFromDirectory(std::string directory_path) 
{
    for (auto& entry : fs::recursive_directory_iterator(directory_path)) {
        if (entry.is_regular_file()) {
            addFile(entry.path().string());
            break;
        }
    }
}

void Index::addFile(std::string path) 
{
    BOOST_LOG_TRIVIAL(info) << path;

    // path_id is index of document_paths_ vector
    int document_id = (int) document_paths_.size();
    document_paths_.push_back(path);

    std::ifstream fin(path);
    if (!fin.bad()) {

        std::streampos word_start = 0;
        std::string word;
        while(!fin.eof()) {

            word_start = fin.tellg();
            fin >> word;

            addTerm(normalizeToken(word), {document_id, word_start});
        }

    } else {
        BOOST_LOG_TRIVIAL(error) << "cannot read file " << path;
    }
}

void Index::addTerm(const std::string& word, TermPosition position) 
{
    auto result = term_positions_.find(word);
    if (result != term_positions_.end()){
        result->second.emplace_back(position);
    } else {
        std::list<TermPosition> positions_new;
        positions_new.emplace_back(position);

        term_positions_.emplace(word, std::move(positions_new));
    }
}

std::string Index::normalizeToken(const std::string& word) 
{
    // preallocate memory for new token
    std::string token;
    token.reserve(word.size());
    std::stringstream ss(std::move(token));

    for (auto& letter : word) {
        if (letter != '.' && 
            letter != ',' && 
            letter != '?' && 
            letter != '!' && 
            letter != '(' && 
            letter != ')')
        {
            ss << std::tolower(letter, std::locale());
        }
    }

    return ss.str();
}
