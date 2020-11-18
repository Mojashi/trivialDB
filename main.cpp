#include <iostream>
#include <vector>
#include "server.hpp"

int main(){
    boost::asio::io_service io_service;
    Server server(io_service, 31234);

    io_service.run();
}