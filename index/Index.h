#ifndef __INDEX_H__
#define __INDEX_H__

#include <string>
#include <list>
#include <map>
#include <vector>
#include <fstream>

#include <boost/serialization/access.hpp>

#include "TermPosition.h"


class Index {
    friend class boost::serialization::access;

public:
    Index() = default;
    Index(size_t total_files);

    void createFromDirectory(const std::string& directory_path);
    void addFile(const std::string& path);

    std::vector<TermPosition> find(const std::string& query) const;

    // contextRaduis controls number of characters displayed around target term
    void displayResults(const std::vector<TermPosition>& positions) const;

    void save(std::string path) const;
    static Index load(std::string path);


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