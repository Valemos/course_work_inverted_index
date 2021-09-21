#pragma once


#include <array>
#include <vector>

class AESEncryption {

public:
    typedef std::array<char, 16> Key128Type;
    static void InitializeLibrary();

    AESEncryption() = default;
    explicit AESEncryption(Key128Type key);

    void SetPrivateKey(AESEncryption::Key128Type key);

    std::vector<char> Encrypt(const std::vector<char> &data);
    std::vector<char> Decrypt(const std::vector<char> &data);

private:
    Key128Type private_key_{};
};



