#include "IndexSearchServer.h"

#include <boost/log/trivial.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "misc/socket_data_exchange.h"


IndexSearchServer::IndexSearchServer(std::filesystem::path index_path, unsigned short port) :
    index_(Index::load(index_path)), listener_(port)
{
    listener_.SetConnectionHandler(
        [this] (auto sock) { HandleClientQuery(std::move(sock)); }
    );
}

void IndexSearchServer::Start()
{
    listener_.Start();
}


void IndexSearchServer::HandleClientQuery(tcp::socket sock) {
    BOOST_LOG_TRIVIAL(info) << sock.remote_endpoint().address().to_string()
                            << ':'
                            << sock.remote_endpoint().port()
                            << " connected";

    try {
        ServerSession session{std::move(sock)};
        session.Authenticate();

        std::string query = session.ReceiveString();
        
        BOOST_LOG_TRIVIAL(debug) << "client query: \"" << query << '"';

        auto results = index_.find(query);

        BOOST_LOG_TRIVIAL(debug) << "results size: " << results.size();

        auto serialized = socket_data_exchange::serialize(results);
        session.Send(serialized.data());

        BOOST_LOG_TRIVIAL(debug) << "results sent";

    } catch (std::exception& err) {
        BOOST_LOG_TRIVIAL(error) << err.what();
    }
}
