#ifndef __SOCKETCONNECTION_H__
#define __SOCKETCONNECTION_H__

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/buffer.hpp>

using boost::asio::ip::tcp;
using boost::asio::io_service;


class SocketListener
{
public:
    SocketListener(unsigned short port);

    void setConnectionHandler(std::function<void(tcp::socket&)> handler);
    void start();
    void acceptNext();

private:
    std::function<void(tcp::socket&)> listen_handler_;

    // bind at start to reuse lambda expression in acceptNext
    std::function<void(const boost::system::error_code &, tcp::socket)> connection_acceptor_;

    io_service service_;
    tcp::endpoint listen_endpoint_;
    tcp::acceptor acceptor_;

    void handleConnection(const boost::system::error_code & err, tcp::socket sock);
};

#endif // __SOCKETCONNECTION_H__