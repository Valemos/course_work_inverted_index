#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "IndexSearchServer.h"


int main(int, char**) {
    boost::log::core::get()->set_filter (boost::log::trivial::severity >= boost::log::trivial::debug);

    try{
        IndexSearchServer server { "./data.index", 40000 };
        server.start();

    } catch (std::exception& err) {
        BOOST_LOG_TRIVIAL(fatal) << err.what() << std::endl;
    }

    return 0;
}
