#include "EncryptedSocketConnection.h"
#include <boost/asio/buffer.hpp>


EncryptedSocketConnection::EncryptedSocketConnection(tcp::socket socket) : socket_(std::move(socket)) {}

void EncryptedSocketConnection::SetParameters(AESEncryption::KeyType key) {
    message_encryption_.SetPrivateKey(key);
}

void EncryptedSocketConnection::SendData(const std::vector<unsigned char> &data) {
    auto encrypted = message_encryption_.Encrypt(data);
    sendWithSize(encrypted);
}

std::vector<unsigned char> EncryptedSocketConnection::ReceiveData() {
    auto encrypted = receiveWithSize();
    return message_encryption_.Decrypt(encrypted);
}

void EncryptedSocketConnection::sendWithSize(const std::vector<unsigned char> &data) {
    // send resulting size of serialized data first
    auto size = data.size();
    socket_.send(boost::asio::buffer(&size, sizeof(size_t)));
    socket_.send(boost::asio::buffer(data.data(), size));
}

std::vector<unsigned char> EncryptedSocketConnection::receiveWithSize() {
    size_t data_size;
    socket_.receive(boost::asio::buffer(&data_size, sizeof(size_t)));

    std::vector<unsigned char> data(data_size, 0);
    socket_.receive(boost::asio::buffer(data, data_size));
    return data;
}
