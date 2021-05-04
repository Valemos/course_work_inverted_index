
#include <iostream>
#include <filesystem>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/map.hpp>

#include "index/Index.h"
#include "index_server/SocketListener.h"

using boost::asio::buffer;


void handleClientQueries(Index* index, tcp::socket sock) {
    BOOST_LOG_TRIVIAL(info) << "accepted connection from " << sock.remote_endpoint().address().to_string();

    std::string query;
    sock.receive(buffer(query));

    BOOST_LOG_TRIVIAL(info) << "client query: \"" << query << '"';

    auto results = index->find(query);
    auto results_paths = index->getFilePaths(results);
    if (!results.empty()) {
        sock.send(buffer(results, results.size()));
    }
    sock.close();
}


fs::path getIndexPath() {
    return "D:\\coding\\c_c++\\concurrent_index_course_work\\test.index";
}


int main(int, char**) {
    boost::log::core::get()->set_filter (boost::log::trivial::severity >= boost::log::trivial::info);

    try{
        [[maybe_unused]] auto index_path = getIndexPath();
        Index index = Index::load(index_path);
        auto index_ptr = &index;

        BOOST_LOG_TRIVIAL(info) << "index loaded from " << index_path;

        SocketListener listener {40000};
        listener.setConnectionHandler(
            [index_ptr] (auto sock) { handleClientQueries(index_ptr, std::move(sock)); }
        );
        

        BOOST_LOG_TRIVIAL(info) << "started listening";
        // thread is waiting for listener to stop
        listener.start();

    } catch (std::exception& err) {
        BOOST_LOG_TRIVIAL(fatal) << err.what() << std::endl;
    }

    return 0;
}
