#include "DHKeyExchange.h"
#include "Errors.h"
#include <stdexcept>
#include <utility>
#include <openssl/ec.h>

DHKeyExchange::DHKeyExchange(size_t secret_length) : secret_length_(secret_length) {
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
    parameters_context_ = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    if(nullptr == parameters_context_) {
        throw std::runtime_error("cannot initialize key generation context");
    }

    // todo read about ECDH generation and rewrite if needed
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
    if(EVP_PKEY_keygen_init(key_context_) <= 0) { ThrowError(); }
    if (EVP_PKEY_keygen(key_context_, &public_key_) <= 0) { ThrowError(); }
}

std::vector<unsigned char> DHKeyExchange::GetPublicKey() {
    // todo rewrite using evp
}

void DHKeyExchange::SetPeerPublicKey(std::vector<unsigned char> bytes) {
    // todo rewrite using evp
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
