#include <iostream>
#include <filesystem>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <boost/asio.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "misc/socket_data_exchange.h"
#include "index/Index.h"
#include "index_server/SocketListener.h"

using boost::asio::buffer;


void handleClientQueries(Index* index, tcp::socket sock) {
    BOOST_LOG_TRIVIAL(info) << "connected " << sock.remote_endpoint().address().to_string();

    while (true) {
        try {
            BOOST_LOG_TRIVIAL(debug) << "before receive";
            
            std::string query = socket_data_exchange::receiveString(sock);
            
            BOOST_LOG_TRIVIAL(debug) << "client query: \"" << query << '"';

            auto results = index->find(query);
            if (!results.empty()) {
                socket_data_exchange::sendSerialized(sock, results);
                BOOST_LOG_TRIVIAL(debug) << "results sent";
            }

        } catch (boost::system::system_error& err) {
            BOOST_LOG_TRIVIAL(error) << err.what();
            break;
        }
    }

    sock.close();
}


fs::path getIndexPath() {
    return "D:\\coding\\c_c++\\concurrent_index_course_work\\test.index";
}


int main(int, char**) {
    boost::log::core::get()->set_filter (boost::log::trivial::severity >= boost::log::trivial::debug);

    try{
        [[maybe_unused]] auto index_path = getIndexPath();
        Index index = Index::load(index_path);
        auto index_ptr = &index;

        BOOST_LOG_TRIVIAL(debug) << "index loaded from " << index_path;

        SocketListener listener {40000};
        listener.setConnectionHandler(
            [index_ptr] (auto sock) { handleClientQueries(index_ptr, std::move(sock)); }
        );
        

        // thread is waiting for listener to stop
        listener.start();

    } catch (std::exception& err) {
        BOOST_LOG_TRIVIAL(fatal) << err.what() << std::endl;
    }

    return 0;
}
