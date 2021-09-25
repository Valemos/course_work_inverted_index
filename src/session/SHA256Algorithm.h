#pragma once

#include <vector>
#include <openssl/sha.h>


class SHA256Algorithm {
public:
    SHA256Algorithm();
    virtual ~SHA256Algorithm();

    std::vector<unsigned char> HashBytes(const std::vector<unsigned char> &bytes);

    // used for continuous hashing
    void Reset();
    void Update(const std::vector<unsigned char> &bytes);
    std::vector<unsigned char> GetFinalHash(); // returns current hash and resets context

private:
    SHA256_CTX *hash_context_{nullptr};
    std::vector<unsigned char> hash_;

    void Init();
    void Finalize();

};



