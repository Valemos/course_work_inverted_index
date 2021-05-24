#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <boost/asio/ip/tcp.hpp>

#include "misc/user_input.h"
#include "IndexSearchClient.h"


int main(int, char**) {
    boost::log::core::get()->set_filter (boost::log::trivial::severity >= boost::log::trivial::debug);

    IndexSearchClient client;

    do {
        try {
            auto address = user_input::promptIpAddress();
            
            while (true) {
                client.connect(address, 40000);

                std::cout << "enter query: " << std::endl;
                auto query = user_input::promptOnce();

                auto results = client.searchIndex(query);
                client.printResults(results);
                
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
