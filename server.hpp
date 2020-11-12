#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include "table.hpp"

namespace asio = boost::asio;
using boost::asio::ip::tcp;

class Session {
	Table* table;
	tcp::iostream socketStream_;
	
   public:
	Session(Table* table, boost::asio::io_service& io_service);
	tcp::iostream& socketStream();
	void start();
};

class Server {
	Table table;
    boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;

	void start_accept();
	void handle_accept(Session* new_session,
					   const boost::system::error_code& error);

   public:
	Server(boost::asio::io_service& io_service, short port);
};