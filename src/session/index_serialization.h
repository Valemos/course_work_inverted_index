#pragma once

#include <string>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

using namespace boost::iostreams;
using boost::asio::ip::tcp;


namespace index_serialization {
    auto boost_archive_flags = boost::archive::archive_flags::no_header |
                                boost::archive::archive_flags::no_tracking;

    template<typename Serializeable>
    std::vector<unsigned char> serialize(const Serializeable &data);

    template<typename Serializeable>
    Serializeable deserialize(const std::vector<unsigned char> &data);
}

template<typename Serializeable>
std::vector<unsigned char> index_serialization::serialize(const Serializeable &data) {
    boost::asio::streambuf buf;
    boost::archive::binary_oarchive archive(buf, boost_archive_flags);
    archive << data;
    std::vector<unsigned char> bytes(buf.size(), 0);
    std::memcpy(bytes.data(), buf.data().data(), buf.size());
    return bytes;
}

template<typename Serializeable>
Serializeable index_serialization::deserialize(const std::vector<unsigned char> &data) {
    basic_array_source<char> input_source((char *) data.data(), data.size());
    stream<basic_array_source<char> > input_stream(input_source);
    boost::archive::binary_iarchive archive(input_stream, boost_archive_flags);

    Serializeable results;
    archive >> results;
    return results;
}