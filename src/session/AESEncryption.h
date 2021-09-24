#pragma once


#include <array>
#include <vector>
#include <openssl/evp.h>
#include <openssl/aes.h>


class AESEncryption {

public:
    static constexpr int KEY_SIZE {32};
    static constexpr int IV_SIZE {AES_BLOCK_SIZE};
    static constexpr int TAG_SIZE {16};

    typedef std::array<unsigned char, KEY_SIZE> KeyType; // 256 bit key
    typedef std::array<unsigned char, IV_SIZE> IVType;

    AESEncryption();
    ~AESEncryption();

    void SetPrivateKey(AESEncryption::KeyType key);
    std::vector<unsigned char> Encrypt(const std::vector<unsigned char> &data);
    std::vector<unsigned char> Decrypt(const std::vector<unsigned char> &data);

private:
    EVP_CIPHER_CTX *cipher_ctx_{nullptr};
    KeyType private_key_{};

    void HandleErrors();
    void EncryptInit(std::array<unsigned char, IV_SIZE> init_vector);
    std::array<unsigned char, TAG_SIZE> GetGCMTag();
    void DecryptInit(AESEncryption::KeyType key, AESEncryption::IVType init_vector);
};



