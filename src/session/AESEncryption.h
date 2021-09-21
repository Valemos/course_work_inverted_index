#pragma once


#include <array>
#include <vector>

class AESEncryption {
public:
    static void InitializeLibrary();

    AESEncryption();
    explicit AESEncryption(std::array<char, 16> key);

    std::vector<char> Encrypt(const std::vector<char> &data);
    std::vector<char> Decrypt(const std::vector<char> &data);

private:
    std::array<char, 16> private_key_;
};



