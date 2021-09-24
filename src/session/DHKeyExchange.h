#pragma once

#include <openssl/evp.h>
#include <vector>

// high level wrapper for openssl Diffie-Hellman key exchange
class DHKeyExchange {
public:
    DHKeyExchange(size_t secret_length);
    ~DHKeyExchange();

    void InitializeParameters();
    void GeneratePublicKey();
    std::vector<unsigned char> GetPublicKey();
    void SetPeerPublicKey(std::vector<unsigned char> bytes);
    void DeriveSharedSecret();

    static void ThrowError();

    const std::vector<unsigned char> & GetSharedSecret();
private:

    size_t secret_length_;
    EVP_PKEY_CTX *parameters_context_{nullptr};
    EVP_PKEY_CTX *key_context_{nullptr};
    EVP_PKEY_CTX *secret_context_{nullptr};

    EVP_PKEY *parameters_{nullptr};
    EVP_PKEY *public_key_{nullptr};
    EVP_PKEY *peer_public_key_{nullptr};

    std::vector<unsigned char> shared_secret_;

    static std::vector<unsigned char> BytesFromPkey(EVP_PKEY *key);

    static EVP_PKEY *PkeyFromBytes(std::vector<unsigned char> vector);
};



