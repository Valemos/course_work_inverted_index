#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <boost/asio.hpp>

#include "misc/user_input.h"
#include "misc/socket_data_exchange.h"
#include "index/TermPosition.h"

using boost::asio::io_service;
using boost::asio::ip::tcp;
using boost::asio::ip::address;
using boost::asio::buffer;


void handleQueriesLoop(tcp::socket& remote) {
    boost::log::core::get()->set_filter (boost::log::trivial::severity >= boost::log::trivial::info);

    do {
        std::cout << "enter query: " << std::endl;
        socket_data_exchange::sendString(remote, user_input::promptOnce());
        BOOST_LOG_TRIVIAL(debug) << "query sent";

        // receive results
        std::vector<TermPosition> results;
        socket_data_exchange::receiveSerialized(remote, results);
        BOOST_LOG_TRIVIAL(debug) << "results received";

        if (!results.empty()) {
            std::cout << "found positions:" << std::endl;
            for (auto& position : results) {
                std::cout << "doc: " << position.document_index
                          << " pos: " << position.term_start << std::endl;
            }
        } else {
            std::cout << "query not found" << std::endl;
        }

        std::cout << "continue search? ([y]/n): ";
    } while (user_input::promptOnce() != "n");
}


int main(int, char**) {
    boost::log::core::get()->set_filter (boost::log::trivial::severity >= boost::log::trivial::debug);

    io_service service;

    // tcp::endpoint server_ep {user_input::promptIpAddress(), 40000};
    tcp::endpoint server_ep {ip::make_address("127.0.0.1"), 40000};
    
    do {
        BOOST_LOG_TRIVIAL(info) << "client trying to connect" << std::endl;

        try {
            tcp::socket sock(service);
            sock.connect(server_ep);
            if (sock.is_open()) {
                handleQueriesLoop(sock);
            } else {
                BOOST_LOG_TRIVIAL(info) << "remote not available" << std::endl;
            }

        } catch (boost::system::system_error& err) {
            BOOST_LOG_TRIVIAL(info) << err.what() << std::endl;
            break;
        }

        std::cout << "reconnect to server? ([y]/n):";
    } while (user_input::promptOnce() != "n");

    return 0;
}
