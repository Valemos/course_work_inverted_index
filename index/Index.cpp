#include "Index.h"

#include <boost/log/trivial.hpp>

#include <iostream>
#include <sstream>
#include <utility>
#include <fstream>
#include <iterator>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>


Index::Index(size_t total_files) 
{
    document_paths_.reserve(total_files);
}

void Index::createFromDirectory(fs::path directory_path) 
{
    for (auto& entry : fs::recursive_directory_iterator(fs::absolute(directory_path))) {
        if (entry.is_regular_file()) {
            addFile(entry.path().string());
        }
    }
}

void Index::addFile(fs::path path) 
{
    BOOST_LOG_TRIVIAL(info) << path;

    // path_id is index of document_paths_ vector
    int document_id = (int) document_paths_.size();
    document_paths_.push_back(path.string());

    std::ifstream fin(path);
    if (!fin.bad()) {

        std::streampos word_start = 0;
        std::string word;
        while(!fin.eof()) {

            word_start = fin.tellg();
            fin >> word;

            addToken(normalizeToken(word), {document_id, word_start});
        }

    } else {
        BOOST_LOG_TRIVIAL(error) << "cannot read file " << path;
    }
}

std::list<TokenPosition> Index::find(const std::string& query) const
{
    auto tokens = tokenizeQuery(query);
    if (tokens.empty()) return {};

    std::list<TokenPosition> search_results;
    bool is_first_token = true;

    for (auto& token : tokens) {

        auto optional_positions = getTokenPositions(token);
        if (optional_positions.has_value()){
            if (is_first_token) {
                search_results = optional_positions.value();
            } else {
                search_results = getListsIntersection(search_results, optional_positions.value());
            }
        }

        if (search_results.empty()) {
            // stop further checks of tokens
            break;
        } 
    }
    
    return search_results;
}

void Index::displayResults(const std::list<TokenPosition>& positions) const
{
    int result_index = 1;
    const std::string* prev_document_path {nullptr};
    for (auto& position : positions) {
        // document path
        auto* cur_document_path = &document_paths_[position.document_index];
        if (cur_document_path != prev_document_path) {
            std::cout << "for path " << *cur_document_path << std::endl;
            prev_document_path = cur_document_path;
        }

        // word position and surroundings
        std::cout << result_index++ << "\t..." 
                << readTermContext(*cur_document_path, position.start, 20) 
                << "..." << std::endl;
    }
}

void Index::save(fs::path path) const
{
    std::ofstream fout {fs::absolute(path), std::ios::binary};
    if (!fout.bad()){
        boost::archive::binary_oarchive archive {fout};
        archive << *this;
    } else {
        throw std::runtime_error("cannot save index to file " + path.string());
    }
}

Index Index::load(fs::path path) 
{
    std::ifstream fin {fs::absolute(path), std::ios::binary};
    if (!fin.bad()){
        boost::archive::binary_iarchive archive {fin};
        Index index;
        archive >> index;
        BOOST_LOG_TRIVIAL(debug) << "index loaded from " << path;

        return index;
        
    } else {
        throw std::runtime_error("cannot read index from file " + path.string());
    }
}

void Index::addToken(const std::string& word, TokenPosition position) 
{
    auto result = token_positions_.find(word);
    if (result != token_positions_.end()){
        result->second.emplace_back(position);
    } else {
        std::list<TokenPosition> positions_new;
        positions_new.emplace_back(position);

        token_positions_.emplace(word, std::move(positions_new));
    }
}

std::string Index::normalizeToken(const std::string& word) const
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

std::string Index::readTermContext(const std::string& file_path, std::streamoff word_start, int context_radius) const
{
    auto read_start = std::max(word_start - context_radius, (std::streamoff) 0);
    auto read_start_size = std::min(word_start, (std::streamoff) context_radius);

    std::stringstream result_stream;

    std::ifstream fin(file_path);
    if (fin.good()) {
        fin.seekg(read_start);

        std::string context;
        context.resize(read_start_size, '\0'); 

        fin.get(&context[0], read_start_size);
        result_stream << context;

        std::string line;
        fin >> line;
        result_stream << line;

        context.replace(0, context.size(), context.size(), '\0');
        context.resize(context_radius, '\0');
        fin.get(&context[0], context_radius);
        result_stream << context;
    }

    return result_stream.str();
}

std::map<int, std::string> Index::getFilePaths(const std::vector<TokenPosition>& positions) const
{
    std::map<int, std::string> paths;
    for (auto& pos : positions) {
        paths.try_emplace(pos.document_index, fs::absolute(document_paths_[pos.document_index]).string());
    }
    return paths;
}

std::optional<std::reference_wrapper<const std::list<TokenPosition>>> Index::getTokenPositions(const std::string& token) const
{
    auto& map_pair = token_positions_.find(token);
    if (map_pair != token_positions_.end()) {
        return map_pair->second;
    }
    return {};
}

std::list<TokenPosition> Index::getListsIntersection(const std::list<TokenPosition>& first, const std::list<TokenPosition>& second) const
{
    // todo: check for correct order of TokenPosition objects
    std::list<TokenPosition> result;
    auto it1 = first.begin(), it2 = second.begin();
    while (it1 != first.end() && it2 != second.end()) {
        if (*it1 < *it2) {
            it1 = std::next(it1);
        } else if (*it2 < *it1) {
            it2 = std::next(it2);
        } else {
            result.emplace_back(*it1);
        }
    }
    return result;
}

std::vector<std::string> Index::tokenizeQuery(const std::string& query) const
{
    std::vector<std::string> tokens;
    size_t last = 0, next = 0; 
    while ((next = query.find(' ', last)) != std::string::npos) {
        tokens.emplace_back(normalizeToken(query.substr(last, next-last)));
    }

    if (tokens.empty()){
        tokens = {normalizeToken(query)};
    }

    return tokens;
}
