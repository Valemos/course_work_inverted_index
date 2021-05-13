#ifndef __INDEX_H__
#define __INDEX_H__

#include <string>
#include <list>
#include <map>
#include <vector>
#include <fstream>
#include <filesystem>
#include <optional>


#include <boost/serialization/access.hpp>
#include "TokenPosition.h"

namespace fs = std::filesystem;


class Index {
    friend class boost::serialization::access;

public:
    Index() = default;
    Index(size_t total_files);

    void createFromDirectory(fs::path directory_path);
    void addFile(fs::path path);
    const std::map<std::string, std::list<TokenPosition>>& getTokenPositions();

    // query must be a single word
    std::list<TokenPosition> find(const std::string& query) const;
    std::map<int, std::string> getFilePaths(const std::list<TokenPosition>& positions) const;

    void displayResults(const std::list<TokenPosition>& positions) const;

    void save(fs::path path) const;
    static Index load(fs::path path);

private:
    std::vector<std::string> document_paths_;
    std::map<std::string, std::list<TokenPosition>> token_positions_;

    // preprocess query
    static std::vector<std::string> tokenizeQuery(const std::string& query);
    static std::string normalizeToken(const std::string& word);

    // add and get tokens with positions 
    void addToken(const std::string& word, TokenPosition position);
    std::optional<std::reference_wrapper<const std::list<TokenPosition>>> getTokenPositions(const std::string& token) const;
    static std::list<TokenPosition> getListsIntersection(
        const std::list<TokenPosition>& first,
        const std::list<TokenPosition>& second
    );

    // read characters around position in indexed file 
    // with context_radius characters before and after first_letter
    std::string readTermContext(const std::string& file_path, std::streamoff first_letter, int context_radius) const;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version);
};


template<class Archive>
void Index::serialize(Archive &ar, const unsigned int version) 
{
    ar & document_paths_;
    ar & token_positions_;
}


#endif // __INDEX_H__