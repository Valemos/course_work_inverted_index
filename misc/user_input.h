#ifndef __USER_INPUT_H__
#define __USER_INPUT_H__


#include <boost/asio/ip/tcp.hpp>
#include <boost/system/system_error.hpp>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;
namespace ip = boost::asio::ip;


namespace user_input {
    std::string promptOnce() noexcept;
    ip::address promptIpAddress() noexcept;
    fs::path promptExistingDirectory() noexcept;
    char promptKeyboardButton() noexcept;
}


std::string user_input::promptOnce() noexcept {
    std::string input;
    std::getline(std::cin, input);
    return input;
}


ip::address user_input::promptIpAddress() noexcept {
    while (true) {
        try {
            std::cout << "enter server ip address:" << std::endl;
            return ip::make_address(promptOnce());

        } catch (boost::system::system_error& err) {
            std::cout << err.what() << std::endl;
        }
    }
}

fs::path user_input::promptExistingDirectory() noexcept {
    while (true) {
        std::cout << "enter directory path:" << std::endl;
        auto path = fs::absolute(promptOnce());
        if (fs::exists(path)) {
            return path;
        }
        std::cout << "path does not exist!" << std::endl;
    }
}

char user_input::promptKeyboardButton() noexcept
{
    char result;
    std::cin.clear();
    std::cin.ignore(10000, '\n');
    std::cin.get(result);
    return result;
}

#endif // __USER_INPUT_H__