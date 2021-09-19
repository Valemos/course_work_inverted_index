#pragma once

#include <string>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/buffer.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

using boost::asio::ip::tcp;


namespace socket_data_exchange {
    auto boost_archive_flags = boost::archive::archive_flags::no_header |
                                boost::archive::archive_flags::no_tracking;

    void sendString(tcp::socket& socket, const std::string& message);
    std::string receiveString(tcp::socket& socket);

    template<typename Serializeable>
    void sendSerialized(tcp::socket& socket, const Serializeable& data);
    
    // receive_size is in bytes and should be a multiple of inner struct elements
    template<typename Serializeable>
    Serializeable receiveSerialized(tcp::socket &socket);

    template<typename Serializeable>
    boost::asio::const_buffer serialize(const Serializeable &data);

    template<typename Serializeable>
    Serializeable deserialize(boost::asio::const_buffer &data);

    void sendWithSize(tcp::socket &socket, boost::asio::streambuf &buf);

    boost::asio::const_buffer receiveWithSize(tcp::socket &socket);
}


void socket_data_exchange::sendString(tcp::socket& socket, const std::string& message) 
{
    std::string appended_message = message + '\n';
    boost::asio::write(socket, boost::asio::buffer(appended_message));
}


std::string socket_data_exchange::receiveString(tcp::socket& socket) 
{
    boost::asio::streambuf buf;
    auto read_size = boost::asio::read_until(socket, buf, "\n");
    std::string result = boost::asio::buffer_cast<const char*>(buf.data());
    return result.substr(0, result.size() - 1);
}

void socket_data_exchange::sendWithSize(tcp::socket &socket,
                                        boost::asio::streambuf &buf) {
    // send resulting size of serialized data first
    auto size = buf.size();
    socket.send(boost::asio::buffer(&size, sizeof(size_t)));

    // send data
    auto bytes_sent = socket.send(buf.data());
    buf.consume(bytes_sent);  // will not work without .consume
}

template<typename Serializeable>
boost::asio::const_buffer socket_data_exchange::serialize(const Serializeable &data) {
    boost::asio::streambuf buf;
    boost::archive::binary_oarchive archive(buf, boost_archive_flags);
    archive << data;
    return buf.data();
}

template<typename Serializeable>
Serializeable socket_data_exchange::receiveSerialized(tcp::socket& socket)
{
    boost::asio::const_buffer buf = receiveWithSize(socket);

    return deserialize<Serializeable>(buf);
}

boost::asio::const_buffer socket_data_exchange::receiveWithSize(tcp::socket &socket) {// get archive size
    size_t data_size;
    socket.receive(boost::asio::buffer(&data_size, sizeof(size_t)));

    // receive archive
    boost::asio::streambuf buf;
    auto bytes_recv = socket.receive(buf.prepare(data_size));
    buf.commit(bytes_recv);
    return buf.data();
}

template<typename Serializeable>
Serializeable socket_data_exchange::deserialize(boost::asio::const_buffer &data) {
    boost::asio::streambuf buf;
    buf.sputn(static_cast<const char *>(data.data()), (std::streamsize)data.size());
    boost::archive::binary_iarchive archive(buf, boost_archive_flags);

    Serializeable results;
    archive >> results;
    return results;
}
