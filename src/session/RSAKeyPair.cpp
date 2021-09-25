#include "RSAKeyPair.h"
#include "SHA256Algorithm.h"
#include <stdexcept>
#include <openssl/rsa.h>
#include <openssl/pem.h>


RSAKeyPair::RSAKeyPair() {
}

RSAKeyPair::~RSAKeyPair() {
    RSA_free(private_key_);
    RSA_free(keys_);
}

void RSAKeyPair::GenerateKeys() {
    EVP_PKEY_CTX *generator_context;
    if(!(generator_context = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr))){
        throw std::runtime_error("cannot initialize key generation generator_context");
    }

    if (EVP_PKEY_keygen_init(generator_context) <= 0)
        throw std::runtime_error("cannot initialize RSA keygen");

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(generator_context, 2048) <= 0)
        throw std::runtime_error("cannot set RSA bits");

    BIGNUM *public_key_exponent = BN_new();
    RSA *key_pair = RSA_new();
    BN_set_word(public_key_exponent, RSA_F4);

    RSA_generate_key_ex(key_pair, 2048, public_key_exponent, NULL);

    keys_ =

    EVP_PKEY_CTX_free(generator_context);
}

void RSAKeyPair::SetPublicKey(const std::string &public_key) {
    RSA *rsa;
    auto c_string = public_key.c_str();
    BIO * keybio = BIO_new_mem_buf((void*)c_string, -1);
    if (keybio == nullptr) {
        throw std::runtime_error("cannot read key string");
    }
    RSA_free(keys_);
    keys_ = PEM_read_bio_RSAPublicKey(keybio, &rsa, nullptr, nullptr);
}

void RSAKeyPair::SetPrivateKey(const std::string &private_key) {
    RSA *rsa;
    auto c_string = private_key.c_str();
    BIO * keybio = BIO_new_mem_buf((void*)c_string, -1);
    if (keybio == nullptr) {
        throw std::runtime_error("cannot read key string");
    }
    RSA_free(private_key_);
    private_key_ = PEM_read_bio_RSAPrivateKey(keybio, &rsa, nullptr, nullptr);
}

std::vector<unsigned char> RSAKeyPair::GetPublicKey() const {
    i2d_RSAPublicKey(keys_, );
}

std::vector<unsigned char> RSAKeyPair::GetPrivateKey() const {
    return KeyToBytes(keys_);
}

std::vector<unsigned char> RSAKeyPair::KeyToBytes(EVP_PKEY *key) {
    std::vector<unsigned char> bytes;
    bytes.resize(i2d_PublicKey(key, 0));
    unsigned char *pp = bytes.data();
    i2d_PublicKey(key, &pp);
    return bytes;
}

EVP_PKEY *RSAKeyPair::BytesToKey(const std::vector<unsigned char> &bytes) {
    EVP_PKEY *new_key;
    const unsigned char *pp = bytes.data();
    d2i_PublicKey(EVP_PKEY_RSA, &new_key, &pp, (long)bytes.size());
    return nullptr;
}

std::vector<unsigned char> RSAKeyPair::Encrypt(const std::vector<unsigned char> &bytes) {
    auto encrypt_context_ =;
    if(1 != EVP_PKEY_encrypt_init(context_))
        throw std::runtime_error("cannot initialize encryption");

    if (EVP_PKEY_encrypt_init(context_) <= 0)
        throw std::runtime_error("cannot initialize encryption");

    if (EVP_PKEY_CTX_set_rsa_padding(context_, RSA_PKCS1_OAEP_PADDING) <= 0)
        throw std::runtime_error("cannot initialize encryption");

    size_t encrypted_len;
    if (1 != EVP_PKEY_encrypt(context_, nullptr, &encrypted_len, bytes.data(), bytes.size()))
        throw std::runtime_error("failed to encrypt");

    std::vector<unsigned char> encrypted;
    encrypted.resize(encrypted_len);


    if (EVP_PKEY_encrypt(context_, encrypted.data(), &encrypted_len, bytes.data(), bytes.size()) <= 0)
        throw std::runtime_error("failed to encrypt");
    encrypted.resize(encrypted_len);

    return encrypted;
}

std::vector<unsigned char> RSAKeyPair::Decrypt(const std::vector<unsigned char> &bytes) {
    if (EVP_PKEY_decrypt_init(context_) <= 0)
        throw std::runtime_error("cannot initialize decryption");

    if (EVP_PKEY_CTX_set_rsa_padding(context_, RSA_PKCS1_OAEP_PADDING) <= 0)
        throw std::runtime_error("cannot initialize decryption");

    size_t decrypted_len;
    if (EVP_PKEY_decrypt(context_, nullptr, &decrypted_len, bytes.data(), bytes.size()) <= 0)
        throw std::runtime_error("failed to decrypt");

    std::vector<unsigned char> decrypted;
    decrypted.resize(decrypted_len);

    if (EVP_PKEY_decrypt(context_, decrypted.data(), &decrypted_len, bytes.data(), bytes.size()) <= 0)
        throw std::runtime_error("failed to decrypt");
    decrypted.resize(decrypted_len);

    return decrypted;
}

std::vector<unsigned char> RSAKeyPair::Sign(const std::vector<unsigned char> &bytes) {

    EVP_MD_CTX* rsa_sign_ctx = EVP_MD_CTX_create();
    EVP_PKEY* priKey  = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(priKey, rsa);
    if (EVP_DigestSignInit(rsa_sign_ctx, NULL, EVP_sha256(), NULL, priKey) <= 0) {
        return false;
    }
    if (EVP_DigestSignUpdate(rsa_sign_ctx, Msg, MsgLen) <= 0) {
        return false;
    }
    if (EVP_DigestSignFinal(rsa_sign_ctx, NULL, MsgLenEnc) <= 0) {
        return false;
    }
    *EncMsg = (unsigned char*)malloc(*MsgLenEnc);
    if (EVP_DigestSignFinal(rsa_sign_ctx, *EncMsg, MsgLenEnc) <= 0) {
        return false;
    }
    EVP_MD_CTX_cleanup(rsa_sign_ctx);
    return true;

    size_t signature_len;
    if (EVP_PKEY_sign(signing_ctx, nullptr, &signature_len, message_digest.data(), message_digest.size()) <= 0)
        throw std::runtime_error("failed to determine signature length");

    std::vector<unsigned char> signature;
    signature.resize(signature_len);

    if (EVP_PKEY_sign(signing_ctx, signature.data(), &signature_len, message_digest.data(), message_digest.size()) <= 0)
        throw std::runtime_error("failed to sign message");

    EVP_PKEY_CTX_free(signing_ctx);
    return signature;
}

bool RSAKeyPair::Verify(const std::vector<unsigned char> &bytes, const std::vector<unsigned char> &signature) {
    EVP_PKEY_CTX* verify_ctx;
    if(!(verify_ctx = EVP_PKEY_CTX_new(keys_, nullptr)))
        throw std::runtime_error("cannot initialize verify context");

    if (EVP_PKEY_verify_init(verify_ctx) <= 0)
        throw std::runtime_error("cannot initialize verify context");

    if (EVP_PKEY_CTX_set_rsa_padding(verify_ctx, RSA_PKCS1_PADDING) <= 0)
        throw std::runtime_error("cannot initialize verify context");

    // TODO check if signature works without digest
    if (EVP_PKEY_CTX_set_signature_md(verify_ctx, EVP_sha256()) <= 0)
        throw std::runtime_error("cannot initialize verify context");

    auto message_digest = SHA256Algorithm().HashBytes(bytes);
    return 1 == EVP_PKEY_verify(verify_ctx, signature.data(), signature.size(), message_digest.data(), message_digest.size());
}
