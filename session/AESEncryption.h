#pragma once


#include <array>
#include <vector>

class AESEncryption {
public:
    static void InitializeLibrary();

    AESEncryption();
    explicit AESEncryption(std::array<char, 16> key);

    const std::vector<char> & Encrypt(std::vector<char> data);
    const std::vector<char> & Decrypt(std::vector<char> data);

private:
    std::array<char, 16> private_key_;
};



