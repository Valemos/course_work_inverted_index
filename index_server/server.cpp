
#include <iostream>
#include <filesystem>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "index/Index.h"


void find(const Index& index, std::string query) {
    auto results = index.find(query);
    if (!results.empty()){
        std::cout << "results:" << std::endl;
        index.displayResults(results);
    } else {
        std::cout << "query not found" << std::endl;
    }
}

fs::path getFilesDirectory() {
    fs::path path;
    while (true) {
        std::cout << "enter path to text files:" << std::endl;
        std::cin >> path;
        path = fs::absolute(path);
        if (fs::exists(path)) {
            return path;
        }

        std::cout << "path not exists!" << std::endl;
    };
} 

int main(int, char**) {
    boost::log::core::get()->set_filter (
        boost::log::trivial::severity >= boost::log::trivial::error
    );

    std::cout << "Server started\n";

    fs::current_path(L"D:\\coding\\c_c++\\concurrent_index_course_work");

    try{
        auto input_path = getFilesDirectory();
        
        Index index(250);
        index.createFromDirectory(input_path);
        // index.createFromDirectory("./datasets/data/aclImdb/test/neg");
        index.save("./test.index");

        // Index index = Index::load("./test.index");
        
        std::string query;
        while (true) {
            std::cout << "enter query (/q to exit):" << std::endl;
            std::cin >> query;
            if (query == "/q") break;
            
            find(index, query);    
        };

    } catch (std::exception& err) {
        std::cout << err.what() << std::endl;
    }

    return 0;
}
