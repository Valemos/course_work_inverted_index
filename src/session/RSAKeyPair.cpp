#include "RSAKeyPair.h"
#include <stdexcept>
#include <iostream>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/params.h>
#include <openssl/param_build.h>
#include <memory>


void show_params(OSSL_PARAM *params) {
    OSSL_PARAM *cur_param;
    int i = 0;
    do {
        cur_param = &params[i++];
    } while (cur_param->key != nullptr);
}

RSAKeyPair::RSAKeyPair() : keys_(nullptr){
}

RSAKeyPair::~RSAKeyPair() {
    EVP_PKEY_free(keys_);
}

void RSAKeyPair::GenerateKeys() {
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (ctx == nullptr)
        throw std::runtime_error("cannot init keygen");

    if (EVP_PKEY_keygen_init(ctx) <= 0)
        throw std::runtime_error("cannot init keygen");

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0)
        throw std::runtime_error("cannot init keygen");

    EVP_PKEY_free(keys_);
    if (1 != EVP_PKEY_generate(ctx, &keys_))
        throw std::runtime_error("cannot generate keys");
}

std::vector<unsigned char> RSAKeyPair::Encrypt(const std::vector<unsigned char> &bytes) {
    auto ctx = EVP_PKEY_CTX_new(keys_, nullptr);
    if (ctx == nullptr)
        throw std::runtime_error("cannot init key context");

    if (EVP_PKEY_encrypt_init(ctx) <= 0)
        throw std::runtime_error("cannot init encryption");

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
        throw std::runtime_error("cannot init encryption");

    size_t encrypted_len;
    if (EVP_PKEY_encrypt(ctx, nullptr, &encrypted_len, bytes.data(), bytes.size()) <= 0)
        throw std::runtime_error("failed to encrypt");

    std::vector<unsigned char> encrypted;
    encrypted.resize(encrypted_len, 0);

    if (EVP_PKEY_encrypt(ctx, encrypted.data(), &encrypted_len, bytes.data(), bytes.size()) <= 0)
        throw std::runtime_error("failed to encrypt");
    encrypted.resize(encrypted_len, 0);

    return encrypted;
}

std::vector<unsigned char> RSAKeyPair::Decrypt(const std::vector<unsigned char> &bytes) {
    auto ctx = EVP_PKEY_CTX_new(keys_, nullptr);
    if (ctx == nullptr)
        throw std::runtime_error("cannot init key context");

    if (EVP_PKEY_decrypt_init(ctx) <= 0)
        throw std::runtime_error("cannot init decryption");

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
        throw std::runtime_error("cannot init decryption");

    size_t decrypted_len;
    if (EVP_PKEY_decrypt(ctx, nullptr, &decrypted_len, bytes.data(), bytes.size()) <= 0)
        throw std::runtime_error("failed to decrypt");

    std::vector<unsigned char> decrypted;
    decrypted.resize(decrypted_len, 0);

    if (EVP_PKEY_decrypt(ctx, decrypted.data(), &decrypted_len, bytes.data(), bytes.size()) <= 0)
        throw std::runtime_error("failed to decrypt");
    decrypted.resize(decrypted_len, 0);

    return decrypted;
}

std::vector<unsigned char> RSAKeyPair::Sign(const std::vector<unsigned char> &bytes) {
    auto md_ctx = EVP_MD_CTX_new();
    auto key_ctx = EVP_PKEY_CTX_new(keys_, nullptr);

    if (EVP_DigestSignInit(md_ctx, &key_ctx, EVP_sha256(), nullptr, keys_) <= 0)
        throw std::runtime_error("cannot init digest and sign context");

    if (EVP_DigestSignUpdate(md_ctx, bytes.data(), bytes.size()) <= 0)
        throw std::runtime_error("cannot update signing digest");

    size_t signature_size;
    if (EVP_DigestSignFinal(md_ctx, nullptr, &signature_size) <= 0)
        throw std::runtime_error("cannot get signature size");

    std::vector<unsigned char> signature;
    signature.resize(signature_size);

    if (EVP_DigestSignFinal(md_ctx, signature.data(), &signature_size) <= 0)
        throw std::runtime_error("cannot sign final digest");
    signature.resize(signature_size);

    return signature;
}

bool RSAKeyPair::Verify(const std::vector<unsigned char> &bytes,
                        const std::vector<unsigned char> &signature) {
    auto md_ctx = EVP_MD_CTX_new();
    auto key_ctx = EVP_PKEY_CTX_new(keys_, nullptr);

    if (EVP_DigestVerifyInit(md_ctx, &key_ctx, EVP_sha256(), nullptr, keys_) <= 0)
        throw std::runtime_error("cannot init digest and verify context");

    if (EVP_DigestVerifyUpdate(md_ctx, bytes.data(), bytes.size()) <= 0)
        throw std::runtime_error("cannot update signing digest");

    return 1 == EVP_DigestVerifyFinal(md_ctx, signature.data(), signature.size());
}

void RSAKeyPair::SetPublicKey(const std::vector<unsigned char> &key) {
    OSSL_PARAM *parameters = GetKeyParameters(&key, nullptr);
    SetKeyFromParameters(parameters);
    OSSL_PARAM_free(parameters);
}

void RSAKeyPair::SetKeyFromParameters(OSSL_PARAM *parameters) {
    auto ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (EVP_PKEY_fromdata_init(ctx) <= 0)
        throw std::runtime_error("cannot init create key");

    EVP_PKEY_free(keys_);
    if (EVP_PKEY_fromdata(ctx, &keys_, EVP_PKEY_KEYPAIR, parameters) <= 0)
        throw std::runtime_error("cannot create key");
}

void RSAKeyPair::SetKeys(const std::vector<unsigned char> &public_key, const std::vector<unsigned char> &private_key) {
    OSSL_PARAM *parameters = GetKeyParameters(&public_key, &private_key);
    SetKeyFromParameters(parameters);
    OSSL_PARAM_free(parameters);
}

OSSL_PARAM *RSAKeyPair::GetKeyParameters(const std::vector<unsigned char> *public_key,
                                         const std::vector<unsigned char> *private_key) {
    auto param_bld = OSSL_PARAM_BLD_new();

    PushParamBignumOrEmpty(param_bld, "n", public_key);

    PushParamBignumOrEmpty(param_bld, "d", private_key);

    // default rsa exponent
    OSSL_PARAM_BLD_push_ulong(param_bld, "e", RSA_F4);

    auto parameters = OSSL_PARAM_BLD_to_param(param_bld);
    OSSL_PARAM_BLD_free(param_bld);

    show_params(parameters);

    return parameters;
}

void RSAKeyPair::PushParamBignumOrEmpty(OSSL_PARAM_BLD *param_bld, const char *key, const std::vector<unsigned char> *data) {
    if (data != nullptr) {
        PushBuildParamBignum(param_bld, key, *data);
    } else {
        std::vector<unsigned char> empty;
        PushBuildParamBignum(param_bld, key, empty);
    }
}

void RSAKeyPair::PushBuildParamBignum(OSSL_PARAM_BLD *param_bld, const char *key, const std::vector<unsigned char> &bytes) {
    BIGNUM *bignum = BN_bin2bn(bytes.data(), (int)bytes.size(), nullptr);
    OSSL_PARAM_BLD_push_BN(param_bld, key, bignum);
}

std::vector<unsigned char> RSAKeyPair::GetPublicKey() {
    return GetNamedParam(keys_, "n");
}

std::vector<unsigned char> RSAKeyPair::GetPrivateKey() {
    return GetNamedParam(keys_, "d");
}

std::vector<unsigned char> RSAKeyPair::GetNamedParam(EVP_PKEY *key, const std::string& name) {
    OSSL_PARAM *param_array;
    if (EVP_PKEY_todata(key, EVP_PKEY_PUBLIC_KEY, &param_array) == 0)
        throw std::runtime_error("cannot read parameters");

    show_params(param_array);

    OSSL_PARAM *cur_param;
    int i = 0;
    do {
        cur_param = &param_array[i++];
        if (std::string(cur_param->key) == name) {
            auto data = GetParameterData(cur_param);
            OSSL_PARAM_free(param_array);
            return data;
        }
    } while (cur_param->key != nullptr);

    throw std::runtime_error("cannot find parameter with name \"" + name + '"');
}

std::vector<unsigned char> RSAKeyPair::GetParameterData(OSSL_PARAM *param) {
    if (OSSL_PARAM_OCTET_STRING == param->data_type) {
        return GetOctetStringData(param);
    } else if (OSSL_PARAM_UNSIGNED_INTEGER == param->data_type){
        return GetBignumData(param);
    }
    throw std::runtime_error("parameter type deserialization was not implemented");
}

std::vector<unsigned char> RSAKeyPair::GetOctetStringData(OSSL_PARAM *param) {
    size_t string_len;
    if(OSSL_PARAM_get_octet_string_ptr(param, nullptr, &string_len) == 0)
        throw std::runtime_error("cannot read string");

    std::vector<unsigned char> bytes;
    bytes.resize(string_len);
    unsigned char *pointer = bytes.data();
    if(OSSL_PARAM_get_octet_string(param, reinterpret_cast<void **>(&pointer), bytes.size(), &string_len) == 0)
        throw std::runtime_error("cannot read string");

    return bytes;
}

std::vector<unsigned char> RSAKeyPair::GetBignumData(OSSL_PARAM *param) {
    BIGNUM *bignum{nullptr};
    if(OSSL_PARAM_get_BN(param, &bignum) == 0)
        throw std::runtime_error("cannot read bignum");

    std::vector<unsigned char> bytes;
    int new_size = BN_num_bytes(bignum);
    bytes.resize(new_size);
    BN_bn2bin(bignum, bytes.data());

    return bytes;
}
