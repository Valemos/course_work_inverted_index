
#ifndef __SEARCH_QUERY_H__
#define __SEARCH_QUERY_H__

#include <string>

#include <boost/asio.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

using boost::asio::ip::tcp;


namespace socked_data_exchange {
    void sendString(tcp::socket& socket, std::string message);
    std::string receiveString(tcp::socket& socket);

    template<typename Serializeable>
    void sendSerialized(tcp::socket& socket, const Serializeable& data);
    
    template<typename Serializeable>
    void receiveSerialized(tcp::socket& socket, Serializeable& results);
}


void sendString(tcp::socket& socket, std::string message) 
{
    boost::asio::write(socket, buffer(message));
}

std::string socked_data_exchange::receiveString(tcp::socket& socket) 
{
    boost::asio::streambuf buf;
    size_t read_size = boost::asio::read_until(socket, buf, "\n");
    return boost::asio::buffer_cast<const char*>(buf.data());
}

template<typename Serializeable>
void socked_data_exchange::sendSerialized(tcp::socket& socket, const Serializeable& data) 
{
    boost::asio::streambuf buf;
    boost::archive::binary_oarchive archive(buf);
    archive << data;
    boost::asio::write(sock, buffer(buf));
}

template<typename Serializeable>
void socked_data_exchange::receiveSerialized(tcp::socket& socket, Serializeable& results) 
{
    boost::asio::streambuf buf;
    boost::asio::read(socket, buffer(buf), boost::asio::transfer_all());
    
    boost::archive::binary_iarchive archive(buf);
    archive >> results;
}

#endif // __SEARCH_QUERY_H__