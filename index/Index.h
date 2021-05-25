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
#include "SearchResult.h"

namespace fs = std::filesystem;


class Index {
    friend class boost::serialization::access;

public:
    Index() = default;

    size_t getTotalFiles() const noexcept;
    const std::map<std::string, std::list<TokenPosition>>& getAllPositions() const noexcept;
    
    // to avoid document_id collisions, must use only one type of addFile function 
    void addFile(const fs::path& path);
    void addFile(const fs::path& path, int document_id);
    
    // invalidates other index in process
    void mergeIndex(Index& other);

    // query must be a set of words separated with spaces or a single word
    std::vector<SearchResult> find(const std::string& query) const;

    void save(fs::path path) const;
    static Index load(fs::path path);

    bool operator==(const Index& other) const noexcept;

private:
    std::map<int, std::string> document_paths_;
    std::map<std::string, std::list<TokenPosition>> token_positions_;

    // preprocess query
    static std::vector<std::string> tokenizeQuery(const std::string& query);
    static std::string normalizeToken(const std::string& word);
    std::vector< std::pair<std::string, std::streamoff> > getFileTokens(const fs::path& path) const;

    // add and get tokens with positions
    void addToken(std::string word, TokenPosition position);
    std::optional<std::reference_wrapper<const std::list<TokenPosition>>> getPositionsForToken(const std::string& token) const;
    
    // get common values for two lists
    static std::list<TokenPosition> getIntersectionByDocument(
        const std::list<TokenPosition>& first,
        const std::list<TokenPosition>& second
    );

    // read characters around position in indexed file 
    // with context_radius characters before and after first_letter
    std::string readTermContext(const TokenPosition& position, int context_radius) const;
    std::string readTermContext(const fs::path& file_path, std::streamoff first_letter, int context_radius) const;
    std::vector<SearchResult> readPositionsContext(const std::list<TokenPosition>& positions) const;

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