#include "IndexSearchServer.h"

#include <boost/log/trivial.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "session/socket_data_exchange.h"


IndexSearchServer::IndexSearchServer(const std::filesystem::path& index_path, unsigned short port) :
        index_(Index::Load(index_path)), listener_(port)
{
    listener_.SetConnectionHandler([this](auto sock) { this->OpenSessionFromSocket(std::move(sock)); });
}

void IndexSearchServer::Start()
{
    listener_.Start();
}

void IndexSearchServer::OpenSessionFromSocket(tcp::socket sock) {
    BOOST_LOG_TRIVIAL(info) << sock.remote_endpoint().address().to_string()
                    << ':'
                    << sock.remote_endpoint().port()
                    << " connection requested";

    ServerSession session{std::move(sock)};
    session.StartCommunication();
    HandleClientSession(session);
}

void IndexSearchServer::HandleClientSession(ServerSession &session) {
    try {
        std::string query = session.ReceiveString();
        BOOST_LOG_TRIVIAL(debug) << "client query: \"" << query << '"';

        auto results = index_.Find(query);
        BOOST_LOG_TRIVIAL(debug) << "results size: " << results.size();

        auto serialized = index_data_exchange::serialize(results);
        session.SendData(serialized);
        BOOST_LOG_TRIVIAL(debug) << "results sent";

    } catch (std::exception& err) {
        BOOST_LOG_TRIVIAL(error) << err.what();
    }
}
