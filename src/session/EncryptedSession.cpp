#include "EncryptedSession.h"
#include <utility>
#include <boost/asio/buffer.hpp>
#include <valarray>


EncryptedSession::EncryptedSession(tcp::socket socket) : socket_(std::move(socket)) {}

void EncryptedSession::SendEncrypted(const std::vector<unsigned char> &data) {
    auto encrypted = message_encryption_.Encrypt(data);
    SendWithSize(encrypted);
}

std::vector<unsigned char> EncryptedSession::ReceiveEncrypted() {
    auto encrypted = ReceiveWithSize();
    return message_encryption_.Decrypt(encrypted);
}

void EncryptedSession::SendWithSize(const std::vector<unsigned char> &data) {
    // send resulting size of serialized data first
    auto size = data.size();
    socket_.send(boost::asio::buffer(&size, sizeof(size_t)));
    SendData(data);
    // todo add data hashing to ensure integrity
}

void EncryptedSession::SendData(const std::vector<unsigned char> &data) {
    socket_.send(boost::asio::buffer(data.data(), data.size()));
}

std::vector<unsigned char> EncryptedSession::ReceiveWithSize() {
    // todo add data hashing to ensure integrity
    size_t data_size;
    socket_.receive(boost::asio::buffer(&data_size, sizeof(size_t)));

    return ReceiveData(data_size);
}

std::vector<unsigned char> EncryptedSession::ReceiveData(size_t data_size) {
    std::vector<unsigned char> data(data_size, 0);
    socket_.receive(boost::asio::buffer(data, data_size));
    return data;
}

void EncryptedSession::StartCommunication() {
//    todo change key exchange process
    DHKeyExchange exchange{AESEncryption::KEY_SIZE};
    exchange.InitializeParameters();
    exchange.GeneratePublicKey();

    // order of sending or accepting public key matters
    auto public_key = exchange.GetPublicKey();
    SendData(public_key);
    exchange.SetPeerPublicKey(ReceiveData(public_key.size()));

    exchange.DeriveSharedSecret();
    auto key_bytes = exchange.GetSharedSecret();
    SetPrivateKey(key_bytes);
}

void EncryptedSession::AcceptCommunication() {
//    todo change key exchange process
    DHKeyExchange exchange{AESEncryption::KEY_SIZE};
    exchange.InitializeParameters();
    exchange.GeneratePublicKey();

    // order of sending or accepting public key matters
    auto public_key = exchange.GetPublicKey();
    exchange.SetPeerPublicKey(ReceiveData(public_key.size()));
    SendData(public_key);

    exchange.DeriveSharedSecret();
    auto key_bytes = exchange.GetSharedSecret();
    SetPrivateKey(key_bytes);
}

void EncryptedSession::SetPrivateKey(std::vector<unsigned char> &key_bytes) {
    AESEncryption::KeyType key_array;
    memcpy(key_array.data(), key_bytes.data(), key_bytes.size());
    message_encryption_.SetPrivateKey(key_array);
}

std::string EncryptedSession::ReceiveString() {
    auto data = ReceiveEncrypted();
    return {reinterpret_cast<const char *>(data.data()), data.size()};
}

void EncryptedSession::SendString(const std::string& string) {
    std::vector<unsigned char> data(string.begin(), string.end());
    SendEncrypted(data);
}
