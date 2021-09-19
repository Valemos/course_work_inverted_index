#pragma once


#include <array>

class ClientSession {
public:
    ClientSession();
    ~ClientSession();

private:
    std::array<char, 16> auth_private_key;
    std::array<char, 16> auth_public_key;

    std::array<char, 16> communication_key;
};



