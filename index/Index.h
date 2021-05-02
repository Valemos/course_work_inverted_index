#ifndef __INDEX_H__
#define __INDEX_H__

#include <string>
#include <list>
#include <map>
#include <vector>
#include <fstream>


class Index {

public:
    Index() = default;
    Index(size_t total_files);

    void createFromDirectory(std::string directory_path);
    void addFile(std::string path);

    void save(std::string path);
    void load(std::string path);

private:
    struct TermPosition {
        int document_index;
        std::streampos word_position;
    };

    std::vector<std::string> document_paths_;
    std::map<std::string, std::list<TermPosition>> term_positions_;

    void addTerm(const std::string& word, TermPosition position);
    std::string normalizeToken(const std::string& word);
};

#endif // __INDEX_H__