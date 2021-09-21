#include "IndexSearchClient.h"

#include <iostream>
#include <boost/log/trivial.hpp>
#include "session/socket_data_exchange.h"


// order of initialization matters
IndexSearchClient::IndexSearchClient() :
    service_(), server_socket_(service_), session_(std::move(server_socket_))
{
}

std::vector<SearchResult> IndexSearchClient::searchIndex(std::string query) 
{
    session_.StartCommunication();
    session_.SendString(query);
    BOOST_LOG_TRIVIAL(debug) << "query sent";
    
    auto data = session_.ReceiveData();
    auto results = socket_data_exchange::deserialize<std::vector<SearchResult>>(data);
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

void IndexSearchClient::Connect(const boost::asio::ip::address& address, unsigned short port)
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
