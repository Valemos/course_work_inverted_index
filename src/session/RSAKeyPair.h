#pragma once

#include <openssl/evp.h>
#include <vector>

class RSAKeyPair {
public:
    RSAKeyPair();
    ~RSAKeyPair();

    void GenerateKeys();

    // keys must be properly generated or else algorithm will not work
    // they must be in PEM format to be read correctly
    void SetPublicKey(const std::string &public_key);
    void SetPrivateKey(const std::string &private_key);
    std::vector<unsigned char> GetPublicKey() const;
    std::vector<unsigned char> GetPrivateKey() const;

    // methods for encryption
    std::vector<unsigned char> Encrypt(const std::vector<unsigned char> &bytes);
    std::vector<unsigned char> Decrypt(const std::vector<unsigned char> &bytes);

    // methods for signature with private key
    std::vector<unsigned char> Sign(const std::vector<unsigned char> &bytes); // returns only signature
    bool Verify(const std::vector<unsigned char> &bytes, const std::vector<unsigned char> &signature);

private:
    //    todo rewrite RSA key management
    RSA *keys_{nullptr};

    static std::vector<unsigned char> KeyToBytes(EVP_PKEY *key) ;
    static EVP_PKEY *BytesToKey(const std::vector<unsigned char> &bytes);
};



