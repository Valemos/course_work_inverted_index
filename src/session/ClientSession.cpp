#include <boost/asio/buffer.hpp>
#include <session/socket_data_exchange.h>
#include "ClientSession.h"


ClientSession::ClientSession(tcp::socket socket) : socket_(std::move(socket)) {

}

void ClientSession::StartCommunication() {

}

void ClientSession::SendString(const std::string& string) {
    std::vector<char> data(string.begin(), string.end());
    SendData(data);
}

void ClientSession::SendData(const std::vector<char> &data) {
    auto encrypted = message_encryption_.Encrypt(data);
    socket_data_exchange::sendWithSize(socket_, encrypted);
}

std::vector<char> ClientSession::ReceiveData() {
    auto encrypted = socket_data_exchange::receiveWithSize(socket_);
    return message_encryption_.Decrypt(encrypted);
}
