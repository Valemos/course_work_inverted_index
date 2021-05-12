#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "IndexSearchServer.h"


fs::path getIndexPath() {
    return "D:\\coding\\c_c++\\concurrent_index_course_work\\test.index";
}


int main(int, char**) {
    boost::log::core::get()->set_filter (boost::log::trivial::severity >= boost::log::trivial::debug);

    try{
        IndexSearchServer server { getIndexPath(), 40000 };
        server.start();

    } catch (std::exception& err) {
        BOOST_LOG_TRIVIAL(fatal) << err.what() << std::endl;
    }

    return 0;
}
