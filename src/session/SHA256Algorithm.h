#pragma once

#include <vector>
#include <openssl/sha.h>


class SHA256Algorithm {
public:
    SHA256Algorithm();
    virtual ~SHA256Algorithm();

    // resets a context and hashes all bytes at once
    std::vector<unsigned char> HashBytes(const std::vector<unsigned char> &bytes);

    // updates current digest
    void Update(const std::vector<unsigned char> &bytes);

    // returns current hash and resets context
    std::vector<unsigned char> GetFinalHash();

private:
    SHA256_CTX *hash_context_{nullptr};
    std::vector<unsigned char> hash_;

    void Init();
    void Finalize();
};



