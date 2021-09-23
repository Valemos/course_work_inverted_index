
#include <iostream>
#include <string>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "misc/user_input.h"
#include "IndexBuilder.h"


int main(int, char**) {
    boost::log::core::get()->set_filter (boost::log::trivial::severity >= boost::log::trivial::debug);
    
    IndexBuilder builder((int) user_input::promptSize("threads number"));
    builder.indexDirectory(user_input::promptExistingDirectory());

    builder.getIndex().Save("../data.index");

    return 0;
}
