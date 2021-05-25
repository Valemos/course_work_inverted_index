#include "IndexSearchClient.h"

#include <iostream>
#include <boost/log/trivial.hpp>
#include "misc/socket_data_exchange.h"


IndexSearchClient::IndexSearchClient() :
    service_(), server_socket_(service_)
{
}

std::vector<SearchResult> IndexSearchClient::searchIndex(std::string query) 
{
    socket_data_exchange::sendString(server_socket_, query);
    BOOST_LOG_TRIVIAL(debug) << "query sent";
    
    std::vector<SearchResult> results;
    socket_data_exchange::receiveSerialized(server_socket_, results);
    BOOST_LOG_TRIVIAL(debug) << "results received";

    return results;
}

void IndexSearchClient::printResults(const std::vector<SearchResult>& results)
{
    if (!results.empty()) {
        std::cout << "found positions:" << std::endl;
        for (auto& result : results) {
            std::cout << "doc: " << result.position.document_id 
                    << " pos: " << result.position.start
                    << " context: " << result.context << std::endl;
        }
    } else {
        std::cout << "query not found" << std::endl;
    }
}

void IndexSearchClient::connect(boost::asio::ip::address address, unsigned short port)
{
    BOOST_LOG_TRIVIAL(debug) << "trying to connect" << std::endl;
    tcp::endpoint server_ep {address, port};
    server_socket_.connect(server_ep);
    BOOST_LOG_TRIVIAL(debug) << "client connected" << std::endl;
}

void IndexSearchClient::disconnect() 
{
    server_socket_.close();
    BOOST_LOG_TRIVIAL(debug) << "client disconnected" << std::endl;
}
