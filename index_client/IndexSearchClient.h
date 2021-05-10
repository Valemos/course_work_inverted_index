#ifndef __INDEXSEARCHCLIENT_H__
#define __INDEXSEARCHCLIENT_H__

#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>

#include "index/TokenPosition.h"



class IndexSearchClient {

public:
    IndexSearchClient();

    void connect(boost::asio::ip::address address, unsigned short port);
    std::vector<TokenPosition> searchIndex(std::string query);

private:
    boost::asio::io_service service_;
    boost::asio::ip::tcp::socket server_socket_;

};

#endif // __INDEXSEARCHCLIENT_H__