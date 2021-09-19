#include "AESEncryption.h"

void AESEncryption::InitializeLibrary() {
// todo write openssl initialization from
// https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption
}

AESEncryption::AESEncryption() {}

AESEncryption::AESEncryption(std::array<char, 16> communication_key) : private_key_(communication_key) {
}

const std::vector<char> & AESEncryption::Encrypt(std::vector<char> data) {
    return std::vector<char>();
}

const std::vector<char> & AESEncryption::Decrypt(std::vector<char> data) {
    return std::vector<char>();
}
