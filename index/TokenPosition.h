#ifndef __TERMPOSITION_H__
#define __TERMPOSITION_H__


#include <fstream>
#include <boost/serialization/access.hpp>

class TokenPosition {

public:

    TokenPosition() : document_index(0), start(0) {};
    TokenPosition(int document_index, std::streamoff start) :
        document_index(document_index), start(start)
    {
    }

    int document_index;
    std::streamoff start;


    bool operator<(const TokenPosition& other) const {
        // document index has priority over token start
        if (document_index > other.document_index){
            return false;
        } else if (document_index == other.document_index) {
            return start < other.start;
        } else {
            return true;
        }
    }

    bool operator==(const TokenPosition& other) const {
        return document_index == other.document_index && start == other.start;
    }

private:
    // serialization logic
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & document_index;
        ar & start;
    }
};

#endif // __TERMPOSITION_H__