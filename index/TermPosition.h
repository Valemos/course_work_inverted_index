#ifndef __TERMPOSITION_H__
#define __TERMPOSITION_H__


#include <fstream>
#include <boost/serialization/access.hpp>

struct TermPosition {
    int document_index;
    std::streamoff term_start;

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & document_index;
        ar & term_start;
    }
};

#endif // __TERMPOSITION_H__