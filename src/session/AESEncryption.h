#pragma once


#include <array>
#include <vector>
#include <openssl/evp.h>
#include <openssl/aes.h>


class AESEncryption {

public:
    static constexpr int KEY_SIZE {32};
    static constexpr int IV_SIZE {AES_BLOCK_SIZE};
    static constexpr int GCM_TAG_SIZE {16};

    typedef std::array<unsigned char, IV_SIZE> IVType;
    typedef std::array<unsigned char, GCM_TAG_SIZE> TagType;

    AESEncryption();
    ~AESEncryption();

    void GenerateKey();
    std::vector<unsigned char> GetPrivateKey();
    void SetPrivateKey(std::vector<unsigned char>::const_iterator bytes_begin,
                       std::vector<unsigned char>::const_iterator bytes_end);
    std::vector<unsigned char> Encrypt(std::vector<unsigned char>::const_iterator bytes_begin,
                                       std::vector<unsigned char>::const_iterator bytes_end);
    std::vector<unsigned char> Decrypt(std::vector<unsigned char>::const_iterator bytes_begin,
                                       std::vector<unsigned char>::const_iterator bytes_end);

private:
    EVP_CIPHER_CTX *cipher_ctx_{nullptr};
    std::vector<unsigned char> private_key_{};

    AESEncryption::TagType GetGCMTag();

    void InitEncrypt(IVType &init_vector);

    void InitDecrypt(IVType &iv);
};



