#include "Index.h"

#include <boost/log/trivial.hpp>

#include <iostream>
#include <utility>
#include <iterator>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>


size_t Index::GetTotalFiles() const noexcept
{
    return document_paths_.size();
}

void Index::AddFile(const fs::path& path)
{
    AddFile(path, (int) document_paths_.size());
}

void Index::AddFile(const fs::path& path, int document_id)
{
    auto file_tokens = GetFileTokens(path);
    BOOST_LOG_TRIVIAL(trace) << "file tokens obtained";
    document_paths_.insert(std::make_pair(document_id, path.string()));
    BOOST_LOG_TRIVIAL(trace) << "inserted new document path";

    for (auto& [token, position] : file_tokens) {
        AddToken(token, {document_id, position});
    }
    BOOST_LOG_TRIVIAL(trace) << "new tokens added";
}

void Index::MergeIndex(Index& other)
{
    document_paths_.insert(other.document_paths_.begin(), other.document_paths_.end());

    for (auto& [other_token, other_positions] : other.token_positions_) {
        auto token_search = token_positions_.find(other_token);
        if (token_search == token_positions_.end()) {
            // create new token
            token_positions_.emplace(other_token, other_positions);
        } else {
            token_search->second = MergeListsByDocument(std::move(token_search->second),
                                                        std::move(other_positions));
        }
    }
}

std::vector<SearchResult> Index::Find(const std::string& query) const
{
    auto tokens = TokenizeQuery(query);
    if (tokens.empty()) return {};

    std::list<TokenPosition> positions_found;
    bool is_first_token = true;

    for (auto& token : tokens) {
        if (token.empty()) continue;
        
        auto token_positions = GetPositionsForToken(token);
        if (token_positions.has_value()){
            if (is_first_token) {
                positions_found = token_positions.value();
                is_first_token = false;
            } else {
                positions_found = GetIntersectionByDocument(positions_found, token_positions.value());
            }
        }

        if (positions_found.empty()) {
            break;
        } 
    }
    
    return ReadPositionsContext(positions_found);
}

void Index::Save(const fs::path& path) const
{
    std::ofstream file_out {fs::absolute(path), std::ios::binary};
    if (file_out.bad()){
        throw std::runtime_error("cannot save index to file " + path.string());
    }

    boost::archive::binary_oarchive archive {file_out};
    archive << *this;
}

Index Index::Load(const fs::path& path)
{
    if (!fs::exists(path)) {
        throw std::runtime_error("file not exists " + path.string());
    }
    
    std::ifstream fin {fs::absolute(path), std::ios::binary};
    if (fin.bad()){
        throw std::runtime_error("cannot read index from file " + path.string());
    }
    
    boost::archive::binary_iarchive archive {fin};
    Index index;
    archive >> index;
    BOOST_LOG_TRIVIAL(info) << "index loaded from " << path;

    return index;
}

void Index::AddToken(std::string word, TokenPosition position)
{
    if (word.empty()) return;

    auto result = token_positions_.find(word);
    if (result == token_positions_.end()){
        token_positions_.insert(std::make_pair(word, std::list<TokenPosition> {position}));
    } else {
        result->second.emplace_back(position);
    }
}

std::vector< std::pair<std::string, std::streamoff> > Index::GetFileTokens(const fs::path& path) const
{
    if (!fs::exists(path)) {
        throw std::runtime_error("path does not exist " + path.string());
    }

    std::ifstream fin(path);
    if (!fin.is_open()) {
        throw std::runtime_error("cannot read file " + path.string());
    }
    
    std::vector< std::pair<std::string, std::streamoff> > file_tokens;
    
    std::streampos word_start = 0;
    std::string word;
    while(!fin.eof()) {

        word_start = fin.tellg();
        fin >> word;

        file_tokens.emplace_back(std::make_pair(NormalizeToken(word), word_start));
    }

    return file_tokens;
}

std::string Index::NormalizeToken(const std::string& word)
{
    std::stringstream ss;

    for (auto& letter : word) {
        if (letter != '.' && 
            letter != ',' && 
            letter != '?' && 
            letter != '!' && 
            letter != '(' && 
            letter != ')' && 
            letter != '"' && 
            letter != '"' && 
            letter != '\'')
        {
            ss << std::tolower(letter, std::locale());
        }
    }

    return ss.str();
}

std::string Index::ReadTermContext(const TokenPosition& position, int context_radius) const
{
    auto id_path = document_paths_.find(position.document_id);
    if (id_path == document_paths_.end()) {
        BOOST_LOG_TRIVIAL(warning) << "file id was not found while reading context from term position";
        return "";
    }

    return ReadTermContext(id_path->second, position.start, context_radius);
}

std::string Index::ReadTermContext(const fs::path& file_path, std::streamoff word_start, int context_radius)
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
    const char fill_char = ' ';
    context.resize(left_context_size, fill_char);

    // read context from left
    fin.get(&context[0], left_context_size);
    result_stream << context;

    // read word
    std::string word;
    fin >> word;
    result_stream << word;

    // read context from right
    context.replace(0, context.size(), context.size(), fill_char);
    context.resize(context_radius, fill_char);
    fin.get(&context[0], context_radius);
    result_stream << context;

    return result_stream.str();
}

std::optional<std::reference_wrapper<const std::list<TokenPosition>>> Index::GetPositionsForToken(const std::string& token) const
{
    auto map_pair = token_positions_.find(token);
    if (map_pair != token_positions_.end()) {
        return map_pair->second;
    }
    return {};
}

std::list<TokenPosition> Index::GetIntersectionByDocument(const std::list<TokenPosition>& first, const std::list<TokenPosition>& second)
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
            result.emplace_back(*it2);
            it1++;
            it2++;
        }
    }
    return result;
}

std::list<TokenPosition> Index::MergeListsByDocument(std::list<TokenPosition> first, std::list<TokenPosition> second)
{
    std::list<TokenPosition> result;

    auto pickDocumentPositions = [&result] (auto& list, auto& iter) {
        auto document_end = FindDocumentEnd(list, iter);
        result.splice(result.end(), list, iter, document_end);
        iter = document_end;
    };

    auto it1 = first.begin(), it2 = second.begin();
    while (it1 != first.end() && it2 != second.end()) {
        if (it1->document_id < it2->document_id) { 
            // first list contains desired document
            pickDocumentPositions(first, it1);
        } else if (it1->document_id > it2->document_id) { 
            // second list contains desired document
            pickDocumentPositions(second, it2);
        } else { 
            // both lists contain elements from the same file
            pickDocumentPositions(first, it1);
            pickDocumentPositions(second, it2);
        }
    }

    if (it1 != first.end()) {
        result.splice(result.end(), first, it1, first.end());
    } else if (it2 != second.end()) {
        result.splice(result.end(), second, it2, second.end());
    }

    return result;
}

std::list<TokenPosition>::iterator Index::FindDocumentEnd(const std::list<TokenPosition> &list, const std::list<TokenPosition>::iterator& start)
{
    auto end = start;

    // skip all elements from the same document
    while (end != list.end()) {
        if (end->document_id == start->document_id){
            end++;
        } else {
            break;
        }
    }

    return end; // return last element from the same document
}

std::vector<std::string> Index::TokenizeQuery(const std::string& query)
{
    std::vector<std::string> tokens;
    size_t start = 0, end = 0; 
    while ((end = query.find(' ', start)) != std::string::npos) {
        tokens.emplace_back(NormalizeToken(query.substr(start, end - start)));
        start = end + 1;
    }

    // the last token will not end with space
    tokens.emplace_back(NormalizeToken(query.substr(start, end)));

    return tokens;
}

bool Index::operator==(const Index& other) const noexcept
{
    for (auto& [id, path_str] : document_paths_) {
        auto search_path = other.document_paths_.find(id);
        if (search_path == other.document_paths_.end()) return false;
        if (fs::absolute(path_str) != fs::absolute(search_path->second)) return false;
    }

    return token_positions_ == other.token_positions_;
}

std::vector<SearchResult> Index::ReadPositionsContext(const std::list<TokenPosition>& positions) const
{
    std::vector<SearchResult> results;
    results.reserve(positions.size());

    for (auto& position : positions) {
        results.emplace_back(SearchResult {position, ReadTermContext(position, 10)});
    }

    return results;
}

const std::map<std::string, std::list<TokenPosition>>& Index::GetAllPositions() const noexcept
{
    return token_positions_;
}
