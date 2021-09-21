#pragma once


#include <array>
#include <boost/asio/ip/tcp.hpp>
#include "AESEncryption.h"

using boost::asio::ip::tcp;

class ServerSession {
public:
    explicit ServerSession(tcp::socket socket);
    ~ServerSession();

    std::string ReceiveString();

    void StartCommunication();

    void Send(const char *data, size_t size);

private:
    tcp::socket socket_;
    AESEncryption message_encryption;
};
