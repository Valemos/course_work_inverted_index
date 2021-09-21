#pragma once


#include <array>
#include <string>
#include <boost/asio/ip/tcp.hpp>
#include "AESEncryption.h"

using boost::asio::ip::tcp;

class ClientSession {
public:
    explicit ClientSession(tcp::socket socket);
    ~ClientSession() = default;

    void StartCommunication();

    void SendString(const std::string& string);
    void SendData(const std::vector<char> &data);
    std::vector<char> ReceiveData();

private:
    tcp::socket socket_;
    AESEncryption message_encryption_;
};



