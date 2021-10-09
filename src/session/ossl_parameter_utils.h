#pragma once


#include <stdexcept>
#include <openssl/param_build.h>


std::vector<unsigned char> getOctetStringData(OSSL_PARAM *param);
std::vector<unsigned char> getParameterData(OSSL_PARAM *param);
std::vector<unsigned char> getBignumData(OSSL_PARAM *param);


std::vector<unsigned char> getNamedParam(EVP_PKEY *key, const std::string& name) {
    OSSL_PARAM *param_array;
    if (EVP_PKEY_todata(key, EVP_PKEY_PUBLIC_KEY, &param_array) == 0)
        throw std::runtime_error("cannot read parameters");

    OSSL_PARAM *cur_param;
    int i = 0;
    do {
        cur_param = &param_array[i++];
        if (std::string(cur_param->key) == name) {
            auto data = getParameterData(cur_param);
            OSSL_PARAM_free(param_array);
            return data;
        }
    } while (cur_param->key != nullptr);

    throw std::runtime_error("cannot find parameter with name \"" + name + '"');
}

std::vector<unsigned char> getOctetStringData(OSSL_PARAM *param) {
    size_t string_len;
    if(OSSL_PARAM_get_octet_string_ptr(param, nullptr, &string_len) == 0)
        throw std::runtime_error("cannot read string");

    std::vector<unsigned char> bytes;
    bytes.resize(string_len);
    unsigned char *pointer = bytes.data();
    if(OSSL_PARAM_get_octet_string(param, reinterpret_cast<void **>(&pointer), bytes.size(), &string_len) == 0)
        throw std::runtime_error("cannot read string");

    return bytes;
}

std::vector<unsigned char> getParameterData(OSSL_PARAM *param) {
    if (OSSL_PARAM_OCTET_STRING == param->data_type) {
        return getOctetStringData(param);
    } else if (OSSL_PARAM_UNSIGNED_INTEGER == param->data_type){
        return getBignumData(param);
    }
    throw std::runtime_error("parameter type deserialization was not implemented");
}

std::vector<unsigned char> getBignumData(OSSL_PARAM *param) {
    BIGNUM *bignum{nullptr};
    if(OSSL_PARAM_get_BN(param, &bignum) == 0)
        throw std::runtime_error("cannot read bignum");

    std::vector<unsigned char> bytes;
    int new_size = BN_num_bytes(bignum);
    bytes.resize(new_size);
    BN_bn2bin(bignum, bytes.data());

    return bytes;
}

void PushBuildParamBignum(OSSL_PARAM_BLD *param_bld, const char *key, const std::vector<unsigned char> &bytes) {
    BIGNUM *bignum = BN_bin2bn(bytes.data(), (int)bytes.size(), nullptr);
    OSSL_PARAM_BLD_push_BN(param_bld, key, bignum);
}
