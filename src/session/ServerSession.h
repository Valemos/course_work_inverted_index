#pragma once

#include <array>
#include <boost/asio/ip/tcp.hpp>
#include "AESEncryption.h"
#include "EncryptedSocketConnection.h"

using boost::asio::ip::tcp;

class ServerSession : public EncryptedSocketConnection {
public:
    explicit ServerSession(tcp::socket socket);

    void StartCommunication();
    std::string ReceiveString();
};
