#include "EncryptedSession.h"
#include "RSAKeyPair.h"
#include "Errors.h"
#include <utility>
#include <boost/asio/buffer.hpp>
#include <valarray>


EncryptedSession::EncryptedSession(tcp::socket socket) : socket_(std::move(socket)) {}

void EncryptedSession::SendEncrypted(const std::vector<unsigned char> &bytes) {
    auto encrypted = message_encryption_.Encrypt(bytes.begin(),  bytes.end());
    SendSizedHashed(encrypted);
}

std::vector<unsigned char> EncryptedSession::ReceiveEncrypted() {
    auto encrypted = ReceiveSizedHashed();
    return message_encryption_.Decrypt(encrypted.begin(),  encrypted.end());
}

void EncryptedSession::SendSizedHashed(const std::vector<unsigned char> &data) {
    // send resulting size of serialized data first
    SendSize(data.size());

    auto hash = hasher_.HashBytes(data);
    SendRaw(hash);
    SendRaw(data);
}

void EncryptedSession::SendSize(size_t size) {
    socket_.send(boost::asio::buffer(&size, sizeof(size_t)));
}

void EncryptedSession::SendRaw(const std::vector<unsigned char> &data) {
    socket_.send(boost::asio::buffer(data.data(), data.size()));
}

std::vector<unsigned char> EncryptedSession::ReceiveSizedHashed() {
    size_t data_size = ReceiveSize();
    auto hash = ReceiveRaw(SHA256Algorithm::HASH_SIZE);
    auto data = ReceiveRaw(data_size);
    if (hasher_.HashBytes(data) != hash) {
        throw std::runtime_error("received incorrect data");
    }
    return data;
}

size_t EncryptedSession::ReceiveSize() {
    size_t data_size;
    socket_.receive(boost::asio::buffer(&data_size, sizeof(size_t)));
    return data_size;
}

std::vector<unsigned char> EncryptedSession::ReceiveRaw(size_t data_size) {
    std::vector<unsigned char> data(data_size, 0);
    socket_.receive(boost::asio::buffer(data, data_size));
    return data;
}

void EncryptedSession::StartCommunication() {
    RSAKeyPair client_rsa;
    client_rsa.GenerateKeys();
    auto server_rsa = RSAKeyPair::LoadPublicKey("./keys_server/id_rsa.pub");

    message_encryption_.GenerateKey();
    auto private_key = message_encryption_.GetPrivateKey();

    auto public_key = client_rsa.GetPublicKey();
    SendRaw(public_key);

    // sign with client's private key and encrypt with server key
    auto signature = client_rsa.SignDigest(private_key.begin(),  private_key.end());
    SendRaw(signature);

    auto encrypted_key = server_rsa.Encrypt(private_key.begin(),  private_key.end());

    SendSize(encrypted_key.size());
    SendRaw(encrypted_key);
}

void EncryptedSession::AcceptCommunication() {
    auto server_rsa = RSAKeyPair::LoadFiles("./keys_server/id_rsa.pub", "./keys_server/id_rsa");

    RSAKeyPair client_certificate;
    auto client_public_key = ReceiveRaw(RSAKeyPair::KEY_SIZE);
    client_certificate.SetPublicKey(client_public_key);

    auto signature = ReceiveRaw(RSAKeyPair::SIGNATURE_SIZE);

    auto message_size = ReceiveSize();
    auto encrypted_key = ReceiveRaw(message_size);
    auto private_key = server_rsa.Decrypt(encrypted_key.begin(), encrypted_key.end());

    if (client_certificate.VerifyDigest(private_key.begin(), private_key.end(), signature.begin(), signature.end())) {
        message_encryption_.SetPrivateKey(private_key.begin(), private_key.end());
    } else {
        throw key_exchange_error("key verification failed");
    }
}

std::string EncryptedSession::ReceiveString() {
    auto data = ReceiveEncrypted();
    return {reinterpret_cast<const char *>(data.data()), data.size()};
}

void EncryptedSession::SendString(const std::string& string) {
    std::vector<unsigned char> data(string.begin(), string.end());
    SendEncrypted(data);
}
