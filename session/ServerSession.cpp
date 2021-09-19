#include "ServerSession.h"

#include <utility>


ServerSession::ServerSession(boost::asio::ip::tcp::socket socket) : socket_(std::move(socket)),
                                                                    message_encryption(std::array<char, 16>()) {
}

ServerSession::~ServerSession() {
    socket_.close();
}

std::string ServerSession::ReceiveString() {
    return std::string();
}

void ServerSession::Authenticate() {

}

void ServerSession::Send(std::vector<char> data) {
    std::vector<char> encrypted = message_encryption.Encrypt(std::move(data));

}
