#include "Index.h"

#include <boost/log/trivial.hpp>

#include <iostream>
#include <sstream>
#include <utility>
#include <fstream>
#include <iterator>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>


Index::Index(size_t total_files) 
{
    reserve(total_files);
}

size_t Index::getTotalFiles() const noexcept
{
    return document_paths_.size();
}

void Index::addFile(const fs::path& path) 
{
    addFile(path, (int) document_paths_.size());
}

void Index::addFile(const fs::path& path, int document_id)
{
    BOOST_LOG_TRIVIAL(info) << path;

    document_paths_.emplace(document_id, path.string());

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

void Index::mergeIndex(const Index& other) 
{
    for (auto& id_path : other.document_paths_) {
        document_paths_.emplace(id_path.first, id_path.second);
    }

    for (auto& [other_token, other_positions] : other.token_positions_) {
        auto token_search = token_positions_.find(other_token);
        if (token_search == token_positions_.end()) {
            // create new token
            token_positions_.emplace(other_token, other_positions);
        } else {
            // insert existing
            token_search->second.insert(token_search->second.end(), other_positions.begin(), other_positions.end());
        }
    }
}

std::vector<SearchResult> Index::find(const std::string& query) const
{
    auto tokens = tokenizeQuery(query);
    if (tokens.empty()) return {};

    std::list<TokenPosition> positions_found;
    bool is_first_token = true;

    for (auto& token : tokens) {

        auto token_positions = getPositionsForToken(token);
        if (token_positions.has_value()){
            if (is_first_token) {
                positions_found = token_positions.value();
                is_first_token = false;
            } else {
                positions_found = getIntersectionByDocument(positions_found, token_positions.value());
            }
        }

        if (positions_found.empty()) {
            // stop further checks of tokens
            break;
        } 
    }
    
    return readPositionsContext(positions_found);
}

void Index::reserve(size_t file_count) 
{
    document_paths_.reserve(file_count);
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

void Index::addToken(std::string word, TokenPosition position) 
{
    if (word.empty()) return;

    auto result = token_positions_.find(word);
    if (result != token_positions_.end()){
        result->second.emplace_back(position);
    } else {
        token_positions_.emplace(word, std::list<TokenPosition> {position});
    }
}

std::string Index::normalizeToken(const std::string& word)
{
    std::stringstream ss;

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

std::string Index::readTermContext(const TokenPosition& position, int context_radius) const
{
    auto id_path = document_paths_.find(position.document_index);
    if (id_path == document_paths_.end()) {
        throw std::runtime_error("Programmer error! cannot find file path with id provided.");
    }

    return readTermContext(id_path->second, position.start, context_radius);
}

std::string Index::readTermContext(const fs::path& file_path, std::streamoff word_start, int context_radius) const
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

std::optional<std::reference_wrapper<const std::list<TokenPosition>>> Index::getPositionsForToken(const std::string& token) const
{
    auto map_pair = token_positions_.find(token);
    if (map_pair != token_positions_.end()) {
        return map_pair->second;
    }
    return {};
}

std::list<TokenPosition> Index::getIntersectionByDocument(const std::list<TokenPosition>& first, const std::list<TokenPosition>& second)
{
    // todo: check for correct order of TokenPosition objects
    std::list<TokenPosition> result;
    auto it1 = first.begin(), it2 = second.begin();
    while (it1 != first.end() && it2 != second.end()) {
        if (it1->document_index < it2->document_index) {
            it1++;
        } else if (it2->document_index < it1->document_index) {
            it2++;
        } else {
            result.emplace_back(*it1);
            if (*it2 != *it1) { result.emplace_back(*it2); }
            it1++;
            it2++;
        }
    }
    return result;
}

std::vector<std::string> Index::tokenizeQuery(const std::string& query)
{
    std::vector<std::string> tokens;
    size_t start = 0, end = 0; 
    while ((end = query.find(' ', start)) != std::string::npos) {
        tokens.emplace_back(normalizeToken(query.substr(start, end - start)));
        start = end + 1;
    }

    // the last token will not end with space
    tokens.emplace_back(normalizeToken(query.substr(start, end)));

    return tokens;
}

bool Index::operator==(const Index& other) const noexcept
{
    return token_positions_ == other.token_positions_;
}

std::vector<SearchResult> Index::readPositionsContext(const std::list<TokenPosition>& positions) const
{
    std::vector<SearchResult> results;
    results.reserve(positions.size());

    for (auto& position : positions) {
        results.emplace_back(SearchResult {position, readTermContext(position, 10)});
    }

    return results;
}
