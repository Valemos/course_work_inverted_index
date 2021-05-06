#ifndef __INDEXSEARCHSERVER_H__
#define __INDEXSEARCHSERVER_H__

#include <filesystem>

#include "index/Index.h"
#include "index_server/SocketListener.h"

#include <boost/asio/ip/tcp.hpp>


using boost::asio::ip::tcp;


class IndexSearchServer {

public:
    IndexSearchServer(std::filesystem::path index_path, unsigned short port);
    
    void start();

private:
    Index index_;
    SocketListener listener_;

    void handleClientQueries(tcp::socket sock);
};

#endif // __INDEXSEARCHSERVER_H__