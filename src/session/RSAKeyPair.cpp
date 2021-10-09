#include "RSAKeyPair.h"
#include "ossl_parameter_utils.h"
#include <stdexcept>
#include <openssl/rsa.h>
#include <openssl/params.h>
#include <openssl/param_build.h>
#include <fstream>


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

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, RSAKeyPair::KEY_SIZE * 8) <= 0)
        throw std::runtime_error("cannot init keygen");

    EVP_PKEY_free(keys_);
    keys_ = nullptr;
    if (1 != EVP_PKEY_generate(ctx, &keys_))
        throw std::runtime_error("cannot generate keys");
}

std::vector<unsigned char> RSAKeyPair::Encrypt(std::vector<unsigned char>::iterator bytes_begin,
                                               std::vector<unsigned char>::iterator bytes_end) {
    auto ctx = EVP_PKEY_CTX_new(keys_, nullptr);
    if (ctx == nullptr)
        throw std::runtime_error("cannot init key context");

    if (EVP_PKEY_encrypt_init(ctx) <= 0)
        throw std::runtime_error("cannot init encryption");

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
        throw std::runtime_error("cannot init encryption");

    size_t encrypted_len;
    long bytes_size = std::distance(bytes_begin, bytes_end);
    if (EVP_PKEY_encrypt(ctx, nullptr, &encrypted_len, &*bytes_begin, bytes_size) <= 0)
        throw std::runtime_error("failed to encrypt");

    std::vector<unsigned char> encrypted;
    encrypted.resize(encrypted_len, 0);

    if (EVP_PKEY_encrypt(ctx, encrypted.data(), &encrypted_len, &*bytes_begin, bytes_size) <= 0)
        throw std::runtime_error("failed to encrypt");
    encrypted.resize(encrypted_len, 0);

    return encrypted;
}

std::vector<unsigned char> RSAKeyPair::Decrypt(std::vector<unsigned char>::iterator bytes_begin,
                                               std::vector<unsigned char>::iterator bytes_end) {
    auto ctx = EVP_PKEY_CTX_new(keys_, nullptr);
    if (ctx == nullptr)
        throw std::runtime_error("cannot init key context");

    if (EVP_PKEY_decrypt_init(ctx) <= 0)
        throw std::runtime_error("cannot init decryption");

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
        throw std::runtime_error("cannot init decryption");

    size_t decrypted_len;
    long bytes_size = std::distance(bytes_begin, bytes_end);
    if (EVP_PKEY_decrypt(ctx, nullptr, &decrypted_len, &*bytes_begin, bytes_size) <= 0)
        throw std::runtime_error("failed to decrypt");

    std::vector<unsigned char> decrypted;
    decrypted.resize(decrypted_len, 0);

    if (EVP_PKEY_decrypt(ctx, decrypted.data(), &decrypted_len, &*bytes_begin, bytes_size) <= 0)
        throw std::runtime_error("failed to decrypt");
    decrypted.resize(decrypted_len, 0);

    return decrypted;
}

std::vector<unsigned char> RSAKeyPair::SignDigest(std::vector<unsigned char>::iterator bytes_begin,
                                                  std::vector<unsigned char>::iterator bytes_end) {
    auto md_ctx = EVP_MD_CTX_new();
    auto key_ctx = EVP_PKEY_CTX_new(keys_, nullptr);

    if (EVP_DigestSignInit(md_ctx, &key_ctx, EVP_sha256(), nullptr, keys_) <= 0)
        throw std::runtime_error("cannot init digest and sign context");

    long bytes_size = std::distance(bytes_begin, bytes_end);
    if (EVP_DigestSignUpdate(md_ctx, &*bytes_begin, bytes_size) <= 0)
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

bool RSAKeyPair::VerifyDigest(std::vector<unsigned char>::iterator bytes_begin,
                              std::vector<unsigned char>::iterator bytes_end,
                              std::vector<unsigned char>::iterator signature_begin,
                              std::vector<unsigned char>::iterator signature_end) {
    auto md_ctx = EVP_MD_CTX_new();
    auto key_ctx = EVP_PKEY_CTX_new(keys_, nullptr);

    if (EVP_DigestVerifyInit(md_ctx, &key_ctx, EVP_sha256(), nullptr, keys_) <= 0)
        throw std::runtime_error("cannot init digest and verify context");

    long bytes_size = std::distance(bytes_begin, bytes_end);
    if (EVP_DigestVerifyUpdate(md_ctx, &*bytes_begin, bytes_size) <= 0)
        throw std::runtime_error("cannot update signing digest");

    long signature_size = std::distance(signature_begin, signature_end);
    return 1 == EVP_DigestVerifyFinal(md_ctx, &*signature_begin, signature_size);
}

void RSAKeyPair::SetPublicKey(const std::vector<unsigned char> &key) {
    OSSL_PARAM *parameters = GetKeyParameters(&key, nullptr);
    SetKeyFromParameters(parameters);
    OSSL_PARAM_free(parameters);
}

void RSAKeyPair::SetPrivateKey(const std::vector<unsigned char> &key) {
    OSSL_PARAM *parameters = GetKeyParameters(nullptr, &key);
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

    if (public_key != nullptr) {
        PushBuildParamBignum(param_bld, "n", *public_key);
    }

    if (private_key != nullptr) {
        PushBuildParamBignum(param_bld, "d", *private_key);
    }

    // default rsa exponent
    OSSL_PARAM_BLD_push_ulong(param_bld, "e", RSA_F4);

    auto parameters = OSSL_PARAM_BLD_to_param(param_bld);
    OSSL_PARAM_BLD_free(param_bld);

    return parameters;
}

std::vector<unsigned char> RSAKeyPair::GetPublicKey() {
    return getNamedParam(keys_, "n");
}

std::vector<unsigned char> RSAKeyPair::GetPrivateKey() {
    return getNamedParam(keys_, "d");
}

RSAKeyPair RSAKeyPair::LoadFiles(const std::filesystem::path &public_path, const std::filesystem::path &private_path) {
    RSAKeyPair key_pair;

    std::vector<unsigned char> public_key = ReadFileBytes(public_path);
    std::vector<unsigned char> private_key = ReadFileBytes(private_path);

    if (private_key.empty() && !public_key.empty()) {
        key_pair.SetPublicKey(public_key);
    } else if (!private_key.empty() && !public_key.empty()) {
        key_pair.SetKeys(public_key, private_key);
    } else {
        throw std::runtime_error("cannot read keys");
    }

    return key_pair;
}

RSAKeyPair RSAKeyPair::LoadPublicKey(const std::filesystem::path &public_key_path) {
    RSAKeyPair key_pair;
    std::vector<unsigned char> public_key = ReadFileBytes(public_key_path);
    key_pair.SetPublicKey(public_key);
    return key_pair;
}

void RSAKeyPair::SaveToDirectory(const std::filesystem::path& directory) {
    std::filesystem::create_directory(directory);
    std::ofstream public_file(directory / "id_rsa.pub");
    for (auto character: GetPublicKey()) {
        public_file << (char) character;
    }
    std::ofstream private_file(directory / "id_rsa");
    for (auto character: GetPrivateKey()) {
        private_file << (char) character;
    }
}

std::vector<unsigned char> RSAKeyPair::ReadFileBytes(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path))
        return {};

    std::vector<unsigned char> bytes;
    bytes.resize(std::filesystem::file_size(path), 0);

    std::ifstream public_file(path);
    public_file.read((char*)bytes.data(), (std::streamsize)bytes.size());

    return bytes;
}
