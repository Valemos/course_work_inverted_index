#pragma once

#include <vector>
#include <memory>
#include <string>
#include <openssl/evp.h>
#include <filesystem>
#include <openssl/sha.h>


class RSAKeyPair {
public:
    static constexpr int KEY_SIZE = 256;
    static constexpr int SIGNATURE_SIZE = 256;

    RSAKeyPair();
    ~RSAKeyPair();

    static RSAKeyPair LoadPublicKey(const std::filesystem::path& public_key_path);
    static RSAKeyPair LoadFiles(const std::filesystem::path &public_path, const std::filesystem::path &private_path);
    void SaveToDirectory(const std::filesystem::path& directory);

    void GenerateKeys();
    void SetPublicKey(const std::vector<unsigned char> &key);
    void SetPrivateKey(const std::vector<unsigned char> &key);
    void SetKeys(const std::vector<unsigned char> &public_key, const std::vector<unsigned char> &private_key);
    std::vector<unsigned char> Encrypt(std::vector<unsigned char>::iterator begin,
                                       std::vector<unsigned char>::iterator end);
    std::vector<unsigned char> Decrypt(std::vector<unsigned char>::iterator begin,
                                       std::vector<unsigned char>::iterator end);

    std::vector<unsigned char> SignDigest(std::vector<unsigned char>::iterator begin,
                                          std::vector<unsigned char>::iterator end);
    bool VerifyDigest(std::vector<unsigned char>::iterator bytes_begin,
                      std::vector<unsigned char>::iterator bytes_end,
                      std::vector<unsigned char>::iterator signature_begin,
                      std::vector<unsigned char>::iterator signature_end);

    std::vector<unsigned char> GetPublicKey();
    std::vector<unsigned char> GetPrivateKey();

private:
    EVP_PKEY *keys_;

    static OSSL_PARAM *GetKeyParameters(const std::vector<unsigned char> *public_key,
                                        const std::vector<unsigned char> *private_key);

    void SetKeyFromParameters(OSSL_PARAM *parameters);

    static std::vector<unsigned char> ReadFileBytes(const std::filesystem::path &path);

};
