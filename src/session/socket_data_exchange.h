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


namespace socket_data_exchange {
    auto boost_archive_flags = boost::archive::archive_flags::no_header |
                                boost::archive::archive_flags::no_tracking;

    template<typename Serializeable>
    std::vector<char> serialize(const Serializeable &data);

    template<typename Serializeable>
    Serializeable deserialize(const std::vector<char> &data);

    void sendWithSize(tcp::socket &socket, const std::vector<char> &data);

    std::vector<char> receiveWithSize(tcp::socket &socket);
}

template<typename Serializeable>
std::vector<char> socket_data_exchange::serialize(const Serializeable &data) {
    boost::asio::streambuf buf;
    boost::archive::binary_oarchive archive(buf, boost_archive_flags);
    archive << data;
    std::vector<char> bytes(buf.size(), 0);
    std::memcpy(bytes.data(), buf.data().data(), buf.size());
    return bytes;
}

template<typename Serializeable>
Serializeable socket_data_exchange::deserialize(const std::vector<char> &data) {
    basic_array_source<char> input_source(data.data(), data.size());
    stream<basic_array_source<char> > input_stream(input_source);
    boost::archive::binary_iarchive archive(input_stream, boost_archive_flags);

    Serializeable results;
    archive >> results;
    return results;
}

void socket_data_exchange::sendWithSize(tcp::socket &socket, const std::vector<char> &data) {
    // send resulting size of serialized data first
    auto size = data.size();
    socket.send(boost::asio::buffer(&size, sizeof(size_t)));

    socket.send(boost::asio::buffer(data.data(), size));
}

std::vector<char> socket_data_exchange::receiveWithSize(tcp::socket &socket) {// get archive size
    size_t data_size;
    socket.receive(boost::asio::buffer(&data_size, sizeof(size_t)));

    // receive archive
    std::vector<char> data(data_size, 0);
    auto bytes_recv = socket.receive(boost::asio::buffer(data, data_size));
    return data;
}
