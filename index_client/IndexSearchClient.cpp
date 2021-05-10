#include "IndexSearchClient.h"

#include <boost/log/trivial.hpp>

#include "misc/socket_data_exchange.h"


IndexSearchClient::IndexSearchClient() :
    service_(), server_socket_(service_)
{
}

std::vector<TokenPosition> IndexSearchClient::searchIndex(std::string query) 
{
    socket_data_exchange::sendString(server_socket_, query);
    BOOST_LOG_TRIVIAL(debug) << "query sent";
    
    std::vector<TokenPosition> results;
    socket_data_exchange::receiveSerialized(server_socket_, results);
    BOOST_LOG_TRIVIAL(debug) << "results received";

    return results;
}

void IndexSearchClient::connect(boost::asio::ip::address address, unsigned short port)
{
    BOOST_LOG_TRIVIAL(debug) << "trying to connect" << std::endl;
    tcp::endpoint server_ep {address, port};
    server_socket_.connect(server_ep);
    BOOST_LOG_TRIVIAL(debug) << "client connected" << std::endl;
}
