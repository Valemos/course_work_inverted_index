#include "EncryptedSocketConnection.h"
#include <session/socket_data_exchange.h>
#include <boost/asio/buffer.hpp>


EncryptedSocketConnection::EncryptedSocketConnection(tcp::socket socket) : socket_(std::move(socket)) {}

void EncryptedSocketConnection::SetPrivateKey(AESEncryption::Key128Type key) {
    message_encryption_.SetPrivateKey(key);
}

void EncryptedSocketConnection::SendData(const std::vector<char> &data) {
    auto encrypted = message_encryption_.Encrypt(data);
    socket_data_exchange::sendWithSize(socket_, encrypted);
}

std::vector<char> EncryptedSocketConnection::ReceiveData() {
    auto encrypted = socket_data_exchange::receiveWithSize(socket_);
    return message_encryption_.Decrypt(encrypted);
}
