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

    size_t GetTotalFiles() const noexcept;
    const std::map<std::string, std::list<TokenPosition>>& GetAllPositions() const noexcept;
    
    // to avoid document_id collisions, must use only one type of addFile function 
    void AddFile(const fs::path& path);
    void AddFile(const fs::path& path, int document_id);
    
    // invalidates other index in process
    void MergeIndex(Index& other);

    // query must be a set of words separated with spaces or a single word
    [[nodiscard]] std::vector<SearchResult> Find(const std::string& query) const;

    void Save(const fs::path& path) const;
    static Index Load(const fs::path& path);

    bool operator==(const Index& other) const noexcept;

private:
    std::map<int, std::string> document_paths_;
    std::map<std::string, std::list<TokenPosition>> token_positions_;

    // preprocess query
    static std::vector<std::string> TokenizeQuery(const std::string& query);
    static std::string NormalizeToken(const std::string& word);
    std::vector< std::pair<std::string, std::streamoff> > GetFileTokens(const fs::path& path) const;

    // add and get tokens with positions
    void AddToken(std::string word, TokenPosition position);
    std::optional<std::reference_wrapper<const std::list<TokenPosition>>> GetPositionsForToken(const std::string& token) const;
    
    // get common values for two lists
    static std::list<TokenPosition> GetIntersectionByDocument(
        const std::list<TokenPosition>& first,
        const std::list<TokenPosition>& second
    );

    // WARNING! invalidates both lists in process 
    static std::list<TokenPosition> MergeListsByDocument(std::list<TokenPosition> first, std::list<TokenPosition> second);
    static std::list<TokenPosition>::iterator FindDocumentEnd(const std::list<TokenPosition> &list, const std::list<TokenPosition>::iterator& start);

    // read characters around position in indexed file 
    // with context_radius characters before and after first_letter
    std::string ReadTermContext(const TokenPosition& position, int context_radius) const;
    static std::string ReadTermContext(const fs::path& file_path, std::streamoff word_start, int context_radius) ;
    std::vector<SearchResult> ReadPositionsContext(const std::list<TokenPosition>& positions) const;

    template<class Archive>
    void serialize(Archive &ar, unsigned int version);
};


template<class Archive>
void Index::serialize(Archive &ar, const unsigned int version) 
{
    ar & document_paths_;
    ar & token_positions_;
}


#endif // __INDEX_H__