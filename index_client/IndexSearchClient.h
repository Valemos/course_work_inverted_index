#pragma once

#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>
#include <session/ClientSession.h>

#include "index/SearchResult.h"


class IndexSearchClient {

public:
    IndexSearchClient();

    void connect(boost::asio::ip::address address, unsigned short port);
    void disconnect();
    std::vector<SearchResult> searchIndex(std::string query);
    static void printResults(const std::vector<SearchResult>& results);

private:
    boost::asio::io_service service_;
    boost::asio::ip::tcp::socket server_socket_;

    ClientSession *session_ {nullptr};
};
