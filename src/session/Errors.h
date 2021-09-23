//
// Created by anton on 22.09.21.
//

#ifndef INVERTED_INDEX_ERRORS_H
#define INVERTED_INDEX_ERRORS_H

#include <stdexcept>

class decryption_error : public std::runtime_error {
public:
    explicit decryption_error(const std::string &arg);
};

#include <array>
#include <vector>
#include <openssl/crypto.h>

#endif //INVERTED_INDEX_ERRORS_H
