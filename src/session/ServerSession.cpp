#include "ServerSession.h"

#include <utility>


ServerSession::ServerSession(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)),
                                                                    message_encryption(std::array<char, 16>()) {
}

ServerSession::~ServerSession() {
    socket_.close();
}

void ServerSession::StartCommunication() {

}

std::string ServerSession::ReceiveString() {
    return std::string();
}

void ServerSession::Send(const char *data, size_t size) {
    std::vector<char> encrypted = message_encryption.Encrypt(data);
}
