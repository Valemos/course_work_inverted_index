#pragma once


#include <array>
#include <boost/asio/ip/tcp.hpp>
#include "AESEncryption.h"

class ServerSession {
public:
    explicit ServerSession(boost::asio::ip::tcp::socket socket);
    ~ServerSession();

    std::string ReceiveString();

    void Authenticate();

    void Send(std::vector<char> data);

private:
    boost::asio::ip::tcp::socket socket_;

    std::array<char, 16> client_public_key;
    AESEncryption message_encryption;
};
