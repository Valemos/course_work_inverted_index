#pragma once


#include <array>
#include <string>
#include <boost/asio/ip/tcp.hpp>
#include <boost/crc.hpp>
#include "AESEncryption.h"
#include "SHA256Algorithm.h"

using boost::asio::ip::tcp;

class EncryptedSession {
public:
    explicit EncryptedSession(tcp::socket socket);
    ~EncryptedSession() = default;

    // client uses this function
    void StartCommunication();
    // server accepts
    void AcceptCommunication();

    void SendEncrypted(const std::vector<unsigned char> &bytes);
    std::vector<unsigned char> ReceiveEncrypted();

    std::string ReceiveString();
    void SendString(const std::string& string);

private:
    tcp::socket socket_;
    AESEncryption message_encryption_;
    SHA256Algorithm hasher_;

    void SendSizedHashed(const std::vector<unsigned char> &data);
    std::vector<unsigned char> ReceiveSizedHashed();
    void SendRaw(const std::vector<unsigned char> &data);
    std::vector<unsigned char> ReceiveRaw(size_t data_size);

    void SendSize(size_t size);

    size_t ReceiveSize();
};



