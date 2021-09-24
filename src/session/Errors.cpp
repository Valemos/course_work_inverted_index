//
// Created by anton on 22.09.21.
//

#include "Errors.h"

decryption_error::decryption_error(const std::string &arg) : runtime_error(arg) {}

key_exchange_error::key_exchange_error(const std::string &arg) : runtime_error(arg) {}
