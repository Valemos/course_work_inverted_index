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

    void SendData(const std::vector<unsigned char> &data);
    std::vector<unsigned char> ReceiveData();

protected:
    tcp::socket socket_;

    void SetParameters(AESEncryption::KeyType key);

private:
    AESEncryption message_encryption_;

    void sendWithSize(const std::vector<unsigned char> &data);
    std::vector<unsigned char> receiveWithSize();
};



