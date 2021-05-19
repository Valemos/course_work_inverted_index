
#include <iostream>
#include <string>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "misc/user_input.h"
#include "IndexBuilder.h"


int main(int, char**) {
    boost::log::core::get()->set_filter (boost::log::trivial::severity >= boost::log::trivial::debug);
    
    IndexBuilder builder(user_input::promptSize("threads number"));
    builder.indexDirectory("datasets/data/aclImdb/test/neg");
    // builder.indexDirectory(user_input::promptExistingDirectory());
    
    // std::cout << "enter save path: " << std::endl;
    // builder.getIndex().save(user_input::promptOnce());

    return 0;
}
