#include <iostream>

#include <boost/log/trivial.hpp>

#include <boost/asio/ip/tcp.hpp>

#include "misc/user_input.h"
#include "IndexSearchClient.h"


int main(int, char**) {
    IndexSearchClient client;

    do {
        try {
            auto address = user_input::promptIpAddress();
            
            while (true) {
                std::cout << "enter query: " << std::endl;
                auto query = user_input::promptOnce();

                client.connect(address, 40000);
                auto results = client.searchIndex(query);
                IndexSearchClient::printResults(results);
                client.disconnect();
            };
        
        } catch (boost::system::system_error& err) {
            client.disconnect();
            BOOST_LOG_TRIVIAL(info) << err.what() << std::endl;
        }

        std::cout << "reconnect to server? ([y]/n):";
    } while (user_input::promptOnce() != "n");

    return 0;
}
