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
    auto file_tokens = getFileTokens(path);
    document_paths_.emplace(document_id, path.string());

    // determine necessary size for new elements
    size_t unique_count = 0;
    for (auto& token_pair : file_tokens) {
        if (!token_positions_.contains(token_pair.first)) {
            unique_count++;
        }
    }

    token_positions_.reserve(token_positions_.size() + unique_count);

    for (auto& [token, position] : file_tokens) {
        addToken(token, {document_id, position});
    }
}

void Index::mergeIndex(Index& other) 
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
            auto& token_list = token_search->second;
            token_list.splice(token_list.end(), other_positions);
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
    if (fout.bad()){
        throw std::runtime_error("cannot save index to file " + path.string());
    }

    boost::archive::binary_oarchive archive {fout};
    archive << *this;
}

Index Index::load(fs::path path) 
{
    std::ifstream fin {fs::absolute(path), std::ios::binary};
    if (fin.bad()){
        throw std::runtime_error("cannot read index from file " + path.string());
    }
    
    boost::archive::binary_iarchive archive {fin};
    Index index;
    archive >> index;
    BOOST_LOG_TRIVIAL(debug) << "index loaded from " << path;

    return index;
}

void Index::addToken(std::string token, TokenPosition position) 
{
    if (token.empty()) return;

    auto result = token_positions_.find(token);
    if (result != token_positions_.end()){
        result->second.emplace_back(position);
    } else {
        token_positions_.insert(std::make_pair(token, std::list<TokenPosition> {position}));
    }
}

std::vector< std::pair<std::string, std::streamoff> > Index::getFileTokens(const fs::path& path) const
{
    if (!fs::exists(path)) {
        throw std::runtime_error("path does not exist " + path.string());
    }

    std::ifstream fin(path);
    if (fin.bad()) {
        throw std::runtime_error("cannot read file " + path.string());
    }
    
    std::vector< std::pair<std::string, std::streamoff> > file_tokens;
    
    std::streampos word_start = 0;
    std::string word;
    while(!fin.eof()) {

        word_start = fin.tellg();
        fin >> word;

        file_tokens.push_back(std::make_pair(normalizeToken(word), word_start));
    }

    return file_tokens;
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
    auto id_path = document_paths_.find(position.document_id);
    if (id_path == document_paths_.end()) {
        BOOST_LOG_TRIVIAL(warning) << "file id was not found while reading context from term position";
        return "";
    }

    return readTermContext(id_path->second, position.start, context_radius);
}

std::string Index::readTermContext(const fs::path& file_path, std::streamoff word_start, int context_radius) const
{
    auto left_context_start = std::max(word_start - context_radius, (std::streamoff) 0);
    auto left_context_size = std::min(word_start, (std::streamoff) context_radius);

    std::stringstream result_stream;

    std::ifstream fin(file_path);
    if (fin.bad()) {
        return ""; 
    }

    fin.seekg(left_context_start);

    std::string context;
    context.resize(left_context_size, '\0'); 

    // read context from left
    fin.get(&context[0], left_context_size);
    result_stream << context;

    // read word
    std::string word;
    fin >> word;
    result_stream << word;

    // read context from right
    context.replace(0, context.size(), context.size(), '\0');
    context.resize(context_radius, '\0');
    fin.get(&context[0], context_radius);
    result_stream << context;

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
        if (it1->document_id < it2->document_id) {
            it1++;
        } else if (it2->document_id < it1->document_id) {
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
    for (auto& [id, path_str] : document_paths_) {
        auto search_path = other.document_paths_.find(id);
        if (search_path == other.document_paths_.end()) {
            // id not found
            return false;
        } else if (fs::absolute(path_str) != fs::absolute(search_path->second)) {
            // paths does not match
            return false;
        }
    }

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

const std::unordered_map<std::string, std::list<TokenPosition>>& Index::getAllPositions() const noexcept
{
    return token_positions_;
}
