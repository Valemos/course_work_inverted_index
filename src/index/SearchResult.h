#ifndef __SEARCHRESULT_H__
#define __SEARCHRESULT_H__

#include <string>
#include "TokenPosition.h"

#include <boost/serialization/access.hpp>

struct SearchResult {
    TokenPosition position;
    std::string context;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & position;
        ar & context;
    }
};

#endif // __SEARCHRESULT_H__