#include "server.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include "utils.hpp"
#include <thread>

using boost::asio::ip::tcp;
using std::string;
using std::endl;

Session::Session(Table* table, boost::asio::io_service& io_service)
	: table(table) {}

tcp::iostream& Session::socketStream() { return socketStream_; }

void Session::start() {
	string s;
	
	tcp::iostream &ss = socketStream_;

	ss << "DB > ";
	while (getline(socketStream_, s)) {
		replace(s.begin(),s.end(), '\n', ' ');
		replace(s.begin(),s.end(), '\r', ' ');

		vector<string> args = split(s, ' ');

		if (args.size() == 0) return;
		
		if (args[0] == "begin") {
			TransactionPtr ts = table->makeTransaction(ss, ss);
			ts->begin();
		} else if (args[0] == "exit") {
			return;
		} else if (args[0] == "help") {
			ss << "begin show exit help" << endl;
		} else if (args[0] == "show") {
			table->showAll();
		} else {
			ss << "unknown operation" << endl;
		}
		ss << "DB > ";
	}
}

Server::Server(boost::asio::io_service& io_service, short port)
	: io_service_(io_service),
	  acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
	table.checkPoint();
	start_accept();
}
void Server::start_accept() {
	Session* new_session = new Session(&table, io_service_);
	acceptor_.async_accept(
		*new_session->socketStream().rdbuf(),
		boost::bind(&Server::handle_accept, this, new_session,
					boost::asio::placeholders::error));
}

void Server::handle_accept(Session* new_session,
						   const boost::system::error_code& error) {
	if (!error) {
		std::cout << "accept" << std::endl;
		std::thread t([&](){
			new_session->start();
		});
		t.detach();
	} else {
		delete new_session;
	}

	start_accept();
}
