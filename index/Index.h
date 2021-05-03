#ifndef __INDEX_H__
#define __INDEX_H__

#include <string>
#include <list>
#include <map>
#include <vector>
#include <fstream>
#include <filesystem>

#include <boost/serialization/access.hpp>
#include "TermPosition.h"

namespace fs = std::filesystem;


class Index {
    friend class boost::serialization::access;

public:
    Index() = default;
    Index(size_t total_files);

    void createFromDirectory(fs::path directory_path);
    void addFile(fs::path path);

    // query must be a single word
    std::vector<TermPosition> find(const std::string& query) const;

    // contextRaduis controls number of characters displayed around target term
    void displayResults(const std::vector<TermPosition>& positions) const;

    void save(fs::path path) const;
    static Index load(fs::path path);

private:
    std::vector<std::string> document_paths_;
    std::map<std::string, std::list<TermPosition>> term_positions_;

    void addTerm(const std::string& word, TermPosition position);
    std::string normalizeToken(const std::string& word) const;
    std::string readTermContext(const std::string& file_path, std::streamoff first_letter, int context_radius) const;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version);
};


template<class Archive>
void Index::serialize(Archive &ar, const unsigned int version) 
{
    ar & document_paths_;
    ar & term_positions_;
}


#endif // __INDEX_H__