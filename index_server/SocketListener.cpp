#include "SocketListener.h"

#include <boost/log/trivial.hpp>
#include <functional>
#include <thread>


SocketListener::SocketListener(unsigned short port) :
    listen_endpoint_(tcp::v4(), port), 
    acceptor_(service_, listen_endpoint_)
{
    connection_acceptor_ = [this](const auto& errcode, auto sock) 
    {
        this->handleConnection(errcode, std::move(sock));
    };
}

void SocketListener::setConnectionHandler(std::function<void(tcp::socket)> handler) 
{
    new_connection_handler_ = handler;
}

void SocketListener::start() 
{
    acceptNext();
    service_.run();
}

void SocketListener::stop() 
{
    service_.stop();
}

void SocketListener::acceptNext() 
{
    acceptor_.async_accept(listen_endpoint_, connection_acceptor_);
}

void SocketListener::handleConnection(const boost::system::error_code & err, tcp::socket sock) {
    if (err) {
        BOOST_LOG_TRIVIAL(error) << "Error occured: " << err.message() << std::endl;
        sock.close();
        acceptNext();
        return;
    }

    std::thread client_thread(new_connection_handler_, std::move(sock));
    client_thread.detach();

    acceptNext();
}
