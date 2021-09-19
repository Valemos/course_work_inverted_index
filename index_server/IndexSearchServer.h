#pragma once

#include <filesystem>

#include <boost/asio/ip/tcp.hpp>
#include "index/Index.h"
#include "SocketListener.h"
#include "session/ServerSession.h"


using boost::asio::ip::tcp;


class IndexSearchServer {

public:
    IndexSearchServer(std::filesystem::path index_path, unsigned short port);

    void Start();

private:
    Index index_;
    SocketListener listener_;

    void HandleClientQuery(tcp::socket sock);
};
