#include "IndexSearchServer.h"

#include <boost/log/trivial.hpp>
#include "session/index_serialization.h"


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

    EncryptedSession session{std::move(sock)};
    session.AcceptCommunication();
    HandleClientSession(session);
}

void IndexSearchServer::HandleClientSession(EncryptedSession &session) {
    try {
        std::string query = session.ReceiveString();
        BOOST_LOG_TRIVIAL(debug) << "client query: \"" << query << '"';

        auto results = index_.Find(query);
        BOOST_LOG_TRIVIAL(debug) << "results size: " << results.size();

        auto serialized = index_serialization::serialize(results);
        session.SendEncrypted(serialized);
        BOOST_LOG_TRIVIAL(debug) << "results sent";

    } catch (std::exception& err) {
        BOOST_LOG_TRIVIAL(error) << err.what();
    }
}
