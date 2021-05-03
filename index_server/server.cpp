
#include <iostream>
#include <filesystem>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/asio/buffer.hpp>

#include "index/Index.h"
#include "index_server/SocketListener.h"


void handleClientQueries(Index* index, tcp::socket& sock) {
    BOOST_LOG_TRIVIAL(info) << "accepted connection from " << sock.remote_endpoint().address().to_string();

    auto results = index->find("alien");
    auto results_paths = index->getFilePaths(results);
    if (!results.empty()) {
        sock.send(boost::asio::buffer(results, results.size()));
        sock.send(boost::asio::buffer(results_paths, results_paths.size()));
    }
}


fs::path getIndexPath() {
    return "D:\\coding\\c_c++\\concurrent_index_course_work\\test.index";
}


int main(int, char**) {
    boost::log::core::get()->set_filter (boost::log::trivial::severity >= boost::log::trivial::error);

    try{
        [[maybe_unused]] auto index_path = getIndexPath();
        
        Index index = Index::load(index_path);
        auto index_ptr = &index;

        BOOST_LOG_TRIVIAL(info) << "index loaded from " << index_path;

        SocketListener listener {40000};
        listener.setConnectionHandler(
            [index_ptr] (auto& sock) { handleClientQueries(index_ptr, sock); }
        );
        
        // to stop listener from other thread, create command thread before start

        BOOST_LOG_TRIVIAL(info) << "started listening";
        listener.start();

    } catch (std::exception& err) {
        BOOST_LOG_TRIVIAL(fatal) << err.what() << std::endl;
    }

    return 0;
}
