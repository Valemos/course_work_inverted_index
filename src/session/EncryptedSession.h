#pragma once


#include <array>
#include <string>
#include <boost/asio/ip/tcp.hpp>
#include "AESEncryption.h"
#include "DHKeyExchange.h"

using boost::asio::ip::tcp;

class EncryptedSession {
public:
    explicit EncryptedSession(tcp::socket socket);
    ~EncryptedSession() = default;

    void StartCommunication();
    void AcceptCommunication();

    void SendEncrypted(const std::vector<unsigned char> &data);
    std::vector<unsigned char> ReceiveEncrypted();

    std::string ReceiveString();
    void SendString(const std::string& string);

private:
    tcp::socket socket_;
    AESEncryption message_encryption_;

    void SetPrivateKey(std::vector<unsigned char> &key_bytes);
    void SendWithSize(const std::vector<unsigned char> &data);
    std::vector<unsigned char> ReceiveWithSize();
    void SendData(const std::vector<unsigned char> &data);
    std::vector<unsigned char> ReceiveData(size_t data_size);
};



