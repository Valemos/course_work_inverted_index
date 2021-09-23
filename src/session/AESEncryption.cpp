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

void AESEncryption::SetPrivateKey(AESEncryption::KeyType key) {
    private_key_ = key;
}

void AESEncryption::EncryptInit(std::array<unsigned char, IV_SIZE> init_vector) {
    if(1 != EVP_EncryptInit_ex(cipher_ctx_, EVP_aes_256_gcm(), nullptr, nullptr, nullptr))
        HandleErrors();

    if(1 != EVP_CIPHER_CTX_ctrl(cipher_ctx_, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr))
        HandleErrors();

    /* Initialise key and IV */
    if(1 != EVP_EncryptInit_ex(cipher_ctx_, nullptr, nullptr, private_key_.data(), init_vector.data()))
        HandleErrors();
}

std::vector<unsigned char> AESEncryption::Encrypt(const std::vector<unsigned char> &plaintext) {
    std::array<unsigned char, IV_SIZE> init_vector{};
    RAND_bytes(init_vector.data(), init_vector.size());
    EncryptInit(init_vector);

    std::vector<unsigned char> ciphertext;
    ciphertext.resize(plaintext.size(), 0);

    // can be called multiple times for streaming data
    int encrypted_len;
    if(1 != EVP_EncryptUpdate(cipher_ctx_, ciphertext.data(), &encrypted_len, plaintext.data(), (int)plaintext.size()))
        HandleErrors();

    ciphertext.resize(encrypted_len + AES_BLOCK_SIZE);
    int final_len;
    if(1 != EVP_EncryptFinal_ex(cipher_ctx_, &*(ciphertext.begin() + encrypted_len), &final_len)) {
        HandleErrors();
    }
    // shrink ciphertext to only contain meaningful elements
    ciphertext.resize(encrypted_len + final_len);

    auto tag = GetGCMTag();

    ciphertext.insert(ciphertext.begin(), init_vector.begin(),  init_vector.end());
    ciphertext.insert(ciphertext.begin() + IV_SIZE, tag.begin(),  tag.end());

    return ciphertext;
}

std::array<unsigned char, AESEncryption::TAG_SIZE> AESEncryption::GetGCMTag() {
    std::array<unsigned char, TAG_SIZE> tag{};
    if(1 != EVP_CIPHER_CTX_ctrl(cipher_ctx_, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag.data()))
        HandleErrors();
    return tag;
}

void AESEncryption::DecryptInit(AESEncryption::KeyType key, AESEncryption::IVType init_vector) {
    if(!EVP_DecryptInit_ex(cipher_ctx_, EVP_aes_256_gcm(), nullptr, nullptr, nullptr))
        HandleErrors();

    if(!EVP_CIPHER_CTX_ctrl(cipher_ctx_, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr))
        HandleErrors();

    if(!EVP_DecryptInit_ex(cipher_ctx_, nullptr, nullptr, key.data(), init_vector.data()))
        HandleErrors();
}

std::vector<unsigned char> AESEncryption::Decrypt(const std::vector<unsigned char> &ciphertext) {
    auto ciphertext_it = ciphertext.begin();

    AESEncryption::IVType iv{};
    std::copy(ciphertext_it, ciphertext_it + IV_SIZE, iv.data());
    ciphertext_it += IV_SIZE;

    std::array<unsigned char, TAG_SIZE> tag{};
    std::copy(ciphertext_it, ciphertext_it + TAG_SIZE, tag.begin());
    ciphertext_it += TAG_SIZE;
    int ciphertext_size = (int)ciphertext.size() - IV_SIZE - TAG_SIZE;

    DecryptInit(private_key_, iv);
    std::vector<unsigned char> plaintext(ciphertext_size, 0);

    int plaintext_len;
    if(!EVP_DecryptUpdate(cipher_ctx_, plaintext.data(), &plaintext_len, &*ciphertext_it, ciphertext_size))
        HandleErrors();

    if(!EVP_CIPHER_CTX_ctrl(cipher_ctx_, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag.data()))
        HandleErrors();

    if (0 >= EVP_DecryptFinal_ex(cipher_ctx_, &*(plaintext.begin() + plaintext_len), &plaintext_len)){
        throw decryption_error("ciphertext is not trustworthy");
    }

    return plaintext;
}

void AESEncryption::HandleErrors() {
    throw std::runtime_error("error occurred during encryption process");
}
