#include "SHA256Algorithm.h"


SHA256Algorithm::SHA256Algorithm() {
    Init();
}

SHA256Algorithm::~SHA256Algorithm() {
    delete hash_context_;
}

void SHA256Algorithm::Init() {
    hash_context_ = new SHA256_CTX;
    SHA256_Init(hash_context_);
    hash_.clear();
    hash_.resize(SHA256_DIGEST_LENGTH);
}

std::vector<unsigned char> SHA256Algorithm::HashBytes(const std::vector<unsigned char> &bytes) {
    Init();
    Update(bytes);
    Finalize();
    return hash_;
}

void SHA256Algorithm::Update(const std::vector<unsigned char> &bytes) {
    if (hash_context_ == nullptr) { Init(); }
    SHA256_Update(hash_context_, bytes.data(), bytes.size());
}

void SHA256Algorithm::Finalize() {
    SHA256_Final(hash_.data(), hash_context_);
    Reset();
}

void SHA256Algorithm::Reset() {
    delete hash_context_;
    hash_context_ = nullptr;
}

std::vector<unsigned char> SHA256Algorithm::GetFinalHash() {
    Finalize();
    return hash_;
}
