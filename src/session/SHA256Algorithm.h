#pragma once

#include <vector>
#include <openssl/sha.h>


class SHA256Algorithm {
public:
    static constexpr int HASH_SIZE = SHA256_DIGEST_LENGTH;

    SHA256Algorithm();
    virtual ~SHA256Algorithm();

    std::vector<unsigned char> HashBytes(const std::vector<unsigned char> &bytes);

    // used for continuous hashing
    void Reset();
    void Update(const std::vector<unsigned char> &bytes);
    std::vector<unsigned char> GetFinalHash(); // returns current hash and resets context

private:
    EVP_MD_CTX *hash_context_{nullptr};
    std::vector<unsigned char> hash_;

    void Finalize();
};



