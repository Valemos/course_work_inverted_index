#include "ServerSession.h"
#include <utility>


ServerSession::ServerSession(tcp::socket socket) : EncryptedSocketConnection(std::move(socket)) {
}

void ServerSession::StartCommunication() {
//    exchange secret keys
//    initialize symmetric message encryption

    std::string key {"0000111122223333"};
    AESEncryption::Key128Type key_array;
    std::memcpy(key_array.data(), key.data(), key.size());
    SetPrivateKey(key_array);
}

std::string ServerSession::ReceiveString() {
    auto data = ReceiveData();
    return {data.data(), data.size()};
}
