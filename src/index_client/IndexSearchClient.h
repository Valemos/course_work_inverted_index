#pragma once

#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>

#include "session/EncryptedSession.h"
#include "index/SearchResult.h"


class IndexSearchClient {

public:
    IndexSearchClient() = default;
    ~IndexSearchClient();

    void Connect(const boost::asio::ip::address& address, unsigned short port);
    void Disconnect();
    std::vector<SearchResult> SearchIndex(const std::string& query);
    static void PrintResults(const std::vector<SearchResult>& results);

private:
    boost::asio::io_service service_{};
    EncryptedSession *session_{nullptr};
};
