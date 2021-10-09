#include <openssl/evp.h>
#include <stdexcept>
#include "SHA256Algorithm.h"


SHA256Algorithm::SHA256Algorithm() {
    hash_context_ = EVP_MD_CTX_create();
    Reset();
}

SHA256Algorithm::~SHA256Algorithm() {
    EVP_MD_CTX_free(hash_context_);
}

std::vector<unsigned char> SHA256Algorithm::HashBytes(const std::vector<unsigned char> &bytes) {
    Update(bytes);
    Finalize();
    return hash_;
}

void SHA256Algorithm::Update(const std::vector<unsigned char> &bytes) {
    EVP_DigestUpdate(hash_context_, bytes.data(), bytes.size());
}

void SHA256Algorithm::Finalize() {
    EVP_DigestFinal_ex(hash_context_, hash_.data(), nullptr);
    Reset();
}

void SHA256Algorithm::Reset() {
    EVP_MD_CTX_reset(hash_context_);
    if (1 != EVP_DigestInit_ex(hash_context_, EVP_sha256(), nullptr))
        throw std::runtime_error("failed to initialize hashing");

    hash_.clear();
    hash_.resize(SHA256Algorithm::HASH_SIZE, 0);
}

std::vector<unsigned char> SHA256Algorithm::GetFinalHash() {
    Finalize();
    return hash_;
}
