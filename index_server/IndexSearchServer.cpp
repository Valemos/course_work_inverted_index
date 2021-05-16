#include "IndexSearchServer.h"

#include <boost/log/trivial.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "misc/socket_data_exchange.h"


IndexSearchServer::IndexSearchServer(std::filesystem::path index_path, unsigned short port) :
    index_(Index::load(index_path)), listener_(port)
{
    listener_.setConnectionHandler(
        [this] (auto sock) { handleClientQueries(std::move(sock)); }
    );
}

void IndexSearchServer::start() 
{
    listener_.start();
}


void IndexSearchServer::handleClientQueries(tcp::socket sock) {
    BOOST_LOG_TRIVIAL(info) << sock.remote_endpoint().address().to_string()
                            << ':'
                            << sock.remote_endpoint().port()
                            << " connected";

    while (true) {
        try {
            BOOST_LOG_TRIVIAL(debug) << "before receive";
            
            std::string query = socket_data_exchange::receiveString(sock);
            
            BOOST_LOG_TRIVIAL(debug) << "client query: \"" << query << '"';

            auto results = index_.find(query);

            BOOST_LOG_TRIVIAL(debug) << "results size: " << results.size();

            socket_data_exchange::sendSerialized(sock, results);
            BOOST_LOG_TRIVIAL(debug) << "results sent";

        } catch (boost::system::system_error& err) {
            BOOST_LOG_TRIVIAL(error) << err.what();
            break;
        }
    }

    sock.close();
}
