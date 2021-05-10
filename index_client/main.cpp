#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "misc/user_input.h"
#include "IndexSearchClient.h"


void showResults(const std::vector<TokenPosition>& results) {
    if (!results.empty()) {
        std::cout << "found positions:" << std::endl;
        for (auto& position : results) {
            std::cout << "doc: " << position.document_index << " pos: " << position.start << std::endl;
        }
    } else {
        std::cout << "query not found" << std::endl;
    }
}


int main(int, char**) {
    boost::log::core::get()->set_filter (boost::log::trivial::severity >= boost::log::trivial::debug);

    IndexSearchClient client;

    do {
        try {
            client.connect(user_input::promptIpAddress(), 40000);
            
            do {
                std::cout << "enter query: " << std::endl;
                auto query = user_input::promptOnce();
                BOOST_LOG_TRIVIAL(debug) << "query sent";

                auto results = client.searchIndex(query);
                showResults(results);

                std::cout << "continue search? ([y]/n): ";
            } while (user_input::promptOnce() != "n");    

        } catch (boost::system::system_error& err) {
            BOOST_LOG_TRIVIAL(info) << err.what() << std::endl;
        }

        std::cout << "reconnect to server? ([y]/n):";
    } while (user_input::promptOnce() != "n");

    return 0;
}
