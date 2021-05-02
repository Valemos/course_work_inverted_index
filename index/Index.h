#ifndef __INDEX_H__
#define __INDEX_H__

#include <string>
#include <list>
#include <map>
#include <vector>


class Index {

public:
    Index() = default;
    Index(size_t total_files);

    void createFromDirectory(std::string directory_path);
    void addFileIndex(std::string path);

    void saveToFile(std::string path);
    void loadFromFile(std::string path);

private:
    struct TermPosition {
        int document_index;
        long word_index;
    };

    std::vector<std::string> document_paths_;
    std::map<std::string, std::list<TermPosition>> term_positions_;



};

#endif // __INDEX_H__