#include "SocketListener.h"

#include <boost/log/trivial.hpp>
#include <functional>
#include <thread>
#include <utility>


SocketListener::SocketListener(unsigned short port) :
    listen_endpoint_(tcp::v4(), port), 
    acceptor_(service_, listen_endpoint_)
{
    connection_acceptor_ = [this](const auto& errcode, auto sock) 
    {
        this->handleConnection(errcode, std::move(sock));
    };
}

void SocketListener::SetConnectionHandler(std::function<void(tcp::socket)> handler)
{
    new_connection_handler_ = std::move(handler);
}

void SocketListener::Start()
{
    AcceptNext();
    service_.run();
}

void SocketListener::AcceptNext()
{
    acceptor_.async_accept(listen_endpoint_, connection_acceptor_);
}

void SocketListener::HandleConnection(const boost::system::error_code & err, tcp::socket sock) {
    if (err) {
        BOOST_LOG_TRIVIAL(error) << "Error occured: " << err.message() << std::endl;
        sock.close();
        AcceptNext();
        return;
    }

    std::thread client_thread(new_connection_handler_, std::move(sock));
    client_thread.detach();

    AcceptNext();
}
