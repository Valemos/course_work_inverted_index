#pragma once


#include <array>
#include <string>
#include <boost/asio/ip/tcp.hpp>
#include "AESEncryption.h"
#include "EncryptedSocketConnection.h"

using boost::asio::ip::tcp;

class ClientSession : public EncryptedSocketConnection {
public:
    explicit ClientSession(tcp::socket socket);
    ~ClientSession() = default;

    void StartCommunication();
    void SendString(const std::string& string);
};



