#pragma once

#include <vector>
#include <optional>
#include <functional>
#include <string>
#include <openssl/evp.h>


class RSAKeyPair {
public:
    RSAKeyPair();
    ~RSAKeyPair();

    void GenerateKeys();
    void SetPublicKey(const std::vector<unsigned char> &key);
    void SetKeys(const std::vector<unsigned char> &public_key, const std::vector<unsigned char> &private_key);
    std::vector<unsigned char> Encrypt(const std::vector<unsigned char> &bytes);
    std::vector<unsigned char> Decrypt(const std::vector<unsigned char> &bytes);

    std::vector<unsigned char> Sign(const std::vector<unsigned char> &bytes);
    bool Verify(const std::vector<unsigned char> &bytes, const std::vector<unsigned char> &signature);

    std::vector<unsigned char> GetPublicKey();
    std::vector<unsigned char> GetPrivateKey();

private:
    EVP_PKEY *keys_;

    static OSSL_PARAM *GetKeyParameters(std::optional<std::reference_wrapper<const std::vector<unsigned char>>> public_key,
                                        std::optional<std::reference_wrapper<const std::vector<unsigned char>>> private_key);

    void SetKeyFromParameters(OSSL_PARAM *parameters);

    static std::vector<unsigned char> GetNamedParam(EVP_PKEY *key, const std::string& name);

    static std::vector<unsigned char> GetParameterData(OSSL_PARAM *param);

    static std::vector<unsigned char> GetOctetStringData(OSSL_PARAM *param);

    static std::vector<unsigned char> GetBignumData(OSSL_PARAM *param);

    static void PushParamBuildBignum(OSSL_PARAM_BLD *param_bld, const char *key,
                                     std::vector<unsigned char> &bytes);
};
