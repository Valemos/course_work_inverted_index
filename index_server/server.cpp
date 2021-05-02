
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

int main(int, char**) {
    boost::log::core::get()->set_filter (
        boost::log::trivial::severity >= boost::log::trivial::info
    );

    std::cout << "Server started\n";

    try{
        std::cout << "enter path to text files:" << std::endl;
        std::string input_path;
        std::cin >> input_path;

        Index index(500);
        index.createFromDirectory(input_path);

        // std::filesystem::current_path("D:/coding/c_c++/concurrent_index_course_work/datasets/data");
        // index.createFromDirectory("./aclImdb/test/neg");
        // index.load("D:/coding/c_c++/concurrent_index_course_work/test.index");
        
        std::string query;
        do {
            std::cout << "enter query (/q to exit):" << std::endl;
            std::cin >> query;
            
            find(index, query);    
        } while(query != "/q");

    } catch (std::exception& err) {
        std::cout << err.what() << std::endl;
    }

    return 0;
}
