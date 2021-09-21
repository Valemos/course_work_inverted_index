#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/buffer.hpp>

using boost::asio::ip::tcp;
using boost::asio::io_service;


class SocketListener
{
public:
    explicit SocketListener(unsigned short port);

    void SetConnectionHandler(std::function<void(tcp::socket)> handler);
    void Start();
    void AcceptNext();

private:
    std::function<void(tcp::socket)> new_connection_handler_;

    // bind at start to reuse lambda expression in acceptNext
    std::function<void(const boost::system::error_code &, tcp::socket)> connection_acceptor_;

    io_service service_;
    tcp::endpoint listen_endpoint_;
    tcp::acceptor acceptor_;

    void HandleConnection(const boost::system::error_code & err, tcp::socket sock);
};
