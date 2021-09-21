#pragma once


#include <array>
#include <string>
#include <boost/asio/ip/tcp.hpp>
#include "AESEncryption.h"

using boost::asio::ip::tcp;

class EncryptedSocketConnection {
public:
    explicit EncryptedSocketConnection(tcp::socket socket);
    ~EncryptedSocketConnection() = default;

    void SendData(const std::vector<char> &data);
    std::vector<char> ReceiveData();

protected:
    tcp::socket socket_;

    void SetPrivateKey(AESEncryption::Key128Type key);

private:
    AESEncryption message_encryption_;
};



