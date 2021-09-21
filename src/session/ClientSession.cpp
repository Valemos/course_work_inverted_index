#include <boost/asio/buffer.hpp>
#include "ClientSession.h"


ClientSession::ClientSession(tcp::socket socket) : EncryptedSocketConnection(std::move(socket)) {
}

void ClientSession::StartCommunication() {
    std::string key {"0000111122223333"};
    AESEncryption::Key128Type key_array;
    std::memcpy(key_array.data(), key.data(), key.size());
    SetPrivateKey(key_array);
}

void ClientSession::SendString(const std::string& string) {
    std::vector<char> data(string.begin(), string.end());
    SendData(data);
}

