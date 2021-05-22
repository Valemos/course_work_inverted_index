#ifndef __TERMPOSITION_H__
#define __TERMPOSITION_H__

#include <fstream>
#include <boost/serialization/access.hpp>

class TokenPosition {

public:

    TokenPosition(int document_id = 0, std::streamoff start = 0) : 
        document_id(document_id), start(start) {}

    int document_id;
    std::streamoff start;


    bool operator<(const TokenPosition& other) const {
        // document index has priority over token start
        if (document_id > other.document_id){
            return false;
        } else if (document_id == other.document_id) {
            return start < other.start;
        } else {
            return true;
        }
    }

    bool operator==(const TokenPosition& other) const {
        return document_id == other.document_id && start == other.start;
    }

private:
    // serialization logic
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & document_id;
        ar & start;
    }
};

#endif // __TERMPOSITION_H__