#ifndef __INDEXBUILDER_H__
#define __INDEXBUILDER_H__

#include <filesystem>
#include <boost/asio/thread_pool.hpp>

#include "Index.h"

namespace fs = std::filesystem;


class IndexBuilder {

public:
    IndexBuilder();
    IndexBuilder(int threads_count);


    void indexDirectory(fs::path directory);
    const Index& getIndex();

private:
    int threads_number_;
    boost::asio::thread_pool builder_pool_;

    Index result_;
};


#endif // __INDEXBUILDER_H__