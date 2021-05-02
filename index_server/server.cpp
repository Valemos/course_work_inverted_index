
#include <iostream>
#include <filesystem>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "index/Index.h"


int main(int, char**) {
    boost::log::core::get()->set_filter (
        boost::log::trivial::severity >= boost::log::trivial::info
    );

    std::cout << "Server started\n";

    try{
        std::filesystem::current_path("D:/coding/c_c++/concurrent_index_course_work/datasets/data");

        Index index(500);
        index.createFromDirectory("./aclImdb/test");

    } catch (std::exception& err) {
        std::cout << err.what() << std::endl;
    }

    return 0;
}
