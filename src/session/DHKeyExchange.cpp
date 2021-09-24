#include "DHKeyExchange.h"
#include "Errors.h"
#include <stdexcept>
#include <utility>
#include <openssl/ec.h>

DHKeyExchange::DHKeyExchange(size_t secret_length) : secret_length_(secret_length) {
    if(!(parameters_context_ = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr)))
        throw std::runtime_error("cannot initialize key generation context");

}

DHKeyExchange::~DHKeyExchange() {
    EVP_PKEY_CTX_free(parameters_context_);
    EVP_PKEY_CTX_free(key_context_);
    EVP_PKEY_CTX_free(secret_context_);

    EVP_PKEY_free(public_key_);
    EVP_PKEY_free(parameters_);
    EVP_PKEY_free(peer_public_key_);
}

void DHKeyExchange::InitializeParameters() {
    /* Create the context for parameter generation */
    if(nullptr == (parameters_context_ = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr))) { ThrowError(); }

    /* Initialise the parameter generation */
    if(1 != EVP_PKEY_paramgen_init(parameters_context_)) { ThrowError(); }

    /* We're going to use the ANSI X9.62 Prime 256v1 curve */
    if(1 != EVP_PKEY_CTX_set_ec_paramgen_curve_nid(parameters_context_, NID_X9_62_prime256v1)) { ThrowError(); }

    /* Create the parameter object params */
    if (!EVP_PKEY_paramgen(parameters_context_, &parameters_)) { ThrowError(); }
}

void DHKeyExchange::GeneratePublicKey() {
    // parameters context must be set from outside, or initialized by Initialize
    if (parameters_ == nullptr) {
        throw key_exchange_error("algorithm parameters was not initialized");
    }
    
    /* Create the context for the key generation */
    if(nullptr == (key_context_ = EVP_PKEY_CTX_new(parameters_, nullptr))) { ThrowError(); }

    /* Generate the key */
    if(1 != EVP_PKEY_keygen_init(key_context_)) { ThrowError(); }
    if (1 != EVP_PKEY_keygen(key_context_, &public_key_)) { ThrowError(); }
}

std::vector<unsigned char> DHKeyExchange::GetPublicKey() {
    int len = i2d_PublicKey(public_key_, 0); // with 0 as second arg it gives length
    std::vector<unsigned char> data(len);
    unsigned char *ptr = data.data();
    i2d_PublicKey(public_key_, &ptr);
    return data;
}

void DHKeyExchange::SetPeerPublicKey(std::vector<unsigned char> bytes) {
    EVP_PKEY_free(peer_public_key_);
    EC_KEY *ec_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    peer_public_key_ = EVP_PKEY_new();
    EVP_PKEY_assign_EC_KEY(peer_public_key_, ec_key);
    unsigned char const *ptr = bytes.data();
    d2i_PublicKey(EVP_PKEY_EC, &peer_public_key_, &ptr, (long)bytes.size());
}

void DHKeyExchange::DeriveSharedSecret() {
    // for this call public key must be sent to peer and peer key must be set
    if (peer_public_key_ == nullptr){
        throw key_exchange_error("incorrect order of key exchange protocol");
    }

    /* Create the context for the shared secret derivation */
    if(nullptr == (secret_context_ = EVP_PKEY_CTX_new(public_key_, nullptr))) { ThrowError(); }

    /* Initialise */
    if(1 != EVP_PKEY_derive_init(secret_context_)) { ThrowError(); }

    /* Provide the peer public key */
    if(1 != EVP_PKEY_derive_set_peer(secret_context_, peer_public_key_)) { ThrowError(); }

    /* Determine buffer length for shared secret */
    if(1 != EVP_PKEY_derive(secret_context_, nullptr, &secret_length_)) { ThrowError(); }

    /* Create the buffer */
    shared_secret_.resize(secret_length_, 0);

    /* Derive the shared secret */
    if(1 != EVP_PKEY_derive(secret_context_, shared_secret_.data(), &secret_length_)) { ThrowError(); }
}

void DHKeyExchange::ThrowError() {
    throw key_exchange_error("error occurred during key exchange process");
}

const std::vector<unsigned char> & DHKeyExchange::GetSharedSecret() {
    if (shared_secret_.empty()) {
        throw key_exchange_error("secret key can be acquired only after DeriveSharedSecret() call");
    }
    return shared_secret_;
}
