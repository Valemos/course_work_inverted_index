#include <cstring>
#include "AESEncryption.h"

void AESEncryption::InitializeLibrary() {
// todo write openssl encryption from
// https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption
}

AESEncryption::AESEncryption() {}

AESEncryption::AESEncryption(std::array<char, 16> communication_key) : private_key_(communication_key) {
}

std::vector<char> AESEncryption::Encrypt(const std::vector<char> &data) {
    return data;
}

std::vector<char> AESEncryption::Decrypt(const std::vector<char> &data) {
    return data;
}
