#include "IndexSearchClient.h"

#include <iostream>
#include <boost/log/trivial.hpp>
#include "session/index_serialization.h"


void IndexSearchClient::Connect(const boost::asio::ip::address& address, unsigned short port)
{
    BOOST_LOG_TRIVIAL(debug) << "trying to connect" << std::endl;
    tcp::endpoint server_ep {address, port};
    tcp::socket new_connection_socket(service_);
    new_connection_socket.connect(server_ep);

    session_ = new EncryptedSession(std::move(new_connection_socket));
    session_->StartCommunication();
    BOOST_LOG_TRIVIAL(debug) << "client connected" << std::endl;
}

void IndexSearchClient::Disconnect()
{
    delete session_;
    BOOST_LOG_TRIVIAL(debug) << "client disconnected" << std::endl;
}

std::vector<SearchResult> IndexSearchClient::SearchIndex(const std::string& query)
{
    if (session_ == nullptr) throw std::runtime_error("connection has not established before usage");

    session_->SendString(query);
    BOOST_LOG_TRIVIAL(debug) << "query sent";

    auto data = session_->ReceiveEncrypted();
    auto results = index_serialization::deserialize<std::vector<SearchResult>>(data);
    BOOST_LOG_TRIVIAL(debug) << "results received";

    return results;
}

void IndexSearchClient::PrintResults(const std::vector<SearchResult>& results)
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

IndexSearchClient::~IndexSearchClient() {
    delete session_;
}
