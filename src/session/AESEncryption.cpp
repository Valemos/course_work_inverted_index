#include "AESEncryption.h"
#include "Errors.h"
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdexcept>
#include <boost/log/trivial.hpp>

// class was made from this example
// https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption


AESEncryption::AESEncryption() {
    if(!(cipher_ctx_ = EVP_CIPHER_CTX_new()))
        throw std::runtime_error("OpenSSL cipher context cannot be initialized");
}

AESEncryption::~AESEncryption() {
    EVP_CIPHER_CTX_free(cipher_ctx_);
}

void AESEncryption::SetPrivateKey(std::vector<unsigned char>::const_iterator bytes_begin,
                                  std::vector<unsigned char>::const_iterator bytes_end) {
    if (std::distance(bytes_begin, bytes_end) != AESEncryption::KEY_SIZE)
        throw std::runtime_error("incorrect AES key size, it should be "
                                    + std::to_string(AESEncryption::KEY_SIZE)
                                    + " bytes");

    private_key_ = std::vector<unsigned char> (bytes_begin, bytes_end);
}

std::vector<unsigned char> AESEncryption::Encrypt(std::vector<unsigned char>::const_iterator bytes_begin,
                                                  std::vector<unsigned char>::const_iterator bytes_end) {
    AESEncryption::IVType init_vector{};
    RAND_bytes(init_vector.data(), init_vector.size());
    InitEncrypt(init_vector);

    std::vector<unsigned char> ciphertext;
    unsigned long plaintext_size = std::distance(bytes_begin, bytes_end);
    ciphertext.resize(plaintext_size, 0);

    // can be called multiple times for streaming data
    int encrypted_len;
    if(1 != EVP_EncryptUpdate(cipher_ctx_, ciphertext.data(), &encrypted_len, &*bytes_begin, (int) plaintext_size))
        throw std::runtime_error("failed to encrypt");

    ciphertext.resize(encrypted_len + AES_BLOCK_SIZE);
    int final_len;
    if(1 != EVP_EncryptFinal_ex(cipher_ctx_, &*(ciphertext.begin() + encrypted_len), &final_len)) {
        throw std::runtime_error("failed to encrypt");
    }
    // shrink ciphertext to only contain meaningful elements
    ciphertext.resize(encrypted_len + final_len);

    auto tag = GetGCMTag();

    ciphertext.insert(ciphertext.begin(), init_vector.begin(),  init_vector.end());
    ciphertext.insert(ciphertext.begin() + IV_SIZE, tag.begin(),  tag.end());

    return ciphertext;
}

void AESEncryption::InitEncrypt(AESEncryption::IVType &init_vector) {
    if(1 != EVP_EncryptInit_ex(cipher_ctx_, EVP_aes_256_gcm(), nullptr, nullptr, nullptr))
        throw std::runtime_error("encryption init failed");

    if(1 != EVP_CIPHER_CTX_ctrl(cipher_ctx_, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr))
        throw std::runtime_error("cannot set init vector length");

    /* Initialise key and IV */
    if(1 != EVP_EncryptInit_ex(cipher_ctx_, nullptr, nullptr, private_key_.data(), init_vector.data()))
        throw std::runtime_error("encryption init failed");
}

std::vector<unsigned char> AESEncryption::Decrypt(std::vector<unsigned char>::const_iterator bytes_begin,
                                                  std::vector<unsigned char>::const_iterator bytes_end) {
    auto ciphertext_it = bytes_begin;

    AESEncryption::IVType iv{};
    std::copy(ciphertext_it, ciphertext_it + IV_SIZE, iv.data());
    ciphertext_it += IV_SIZE;

    std::array<unsigned char, GCM_TAG_SIZE> tag{};
    std::copy(ciphertext_it, ciphertext_it + GCM_TAG_SIZE, tag.begin());
    ciphertext_it += GCM_TAG_SIZE;
    int ciphertext_size = (int)std::distance(bytes_begin, bytes_end) - IV_SIZE - GCM_TAG_SIZE;

    InitDecrypt(iv);

    std::vector<unsigned char> plaintext(ciphertext_size, 0);

    int plaintext_len;
    if(1 != EVP_DecryptUpdate(cipher_ctx_, plaintext.data(), &plaintext_len, &*ciphertext_it, ciphertext_size))
        throw std::runtime_error("failed to decrypt");

    if(1 != EVP_CIPHER_CTX_ctrl(cipher_ctx_, EVP_CTRL_GCM_SET_TAG, GCM_TAG_SIZE, tag.data()))
        throw std::runtime_error("cannot set gcm tag");

    if (0 >= EVP_DecryptFinal_ex(cipher_ctx_, &*(plaintext.begin() + plaintext_len), &plaintext_len))
        throw decryption_error("ciphertext is not trustworthy");

    return plaintext;
}

void AESEncryption::InitDecrypt(AESEncryption::IVType &iv) {
    if(1 != EVP_DecryptInit_ex(cipher_ctx_, EVP_aes_256_gcm(), nullptr, nullptr, nullptr))
        throw std::runtime_error("failed to init decryption");

    if(1 != EVP_CIPHER_CTX_ctrl(cipher_ctx_, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr))
        throw std::runtime_error("cannot set init vector length");

    if(1 != EVP_DecryptInit_ex(cipher_ctx_, nullptr, nullptr, private_key_.data(), iv.data()))
        throw std::runtime_error("failed to init decryption");
}

AESEncryption::TagType AESEncryption::GetGCMTag() {
    AESEncryption::TagType tag{};
    if(1 != EVP_CIPHER_CTX_ctrl(cipher_ctx_, EVP_CTRL_GCM_GET_TAG, GCM_TAG_SIZE, tag.data()))
        throw std::runtime_error("cannot get gcm tag");
    return tag;
}

void AESEncryption::GenerateKey() {
    private_key_.resize(AESEncryption::KEY_SIZE);
    RAND_bytes(private_key_.data(), (int)private_key_.size());
}

std::vector<unsigned char> AESEncryption::GetPrivateKey() {
    return private_key_;
}
