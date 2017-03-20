#pragma once
#include "../smartoffice-srv.h"
#include <string>
#include <cstring>
#include <memory>
#include <boost/asio.hpp>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <iostream>
#include <mysql_handler.h>
#include <boost/array.hpp>
#include "mysql_handler.h"
#include "client.h"
#include "session.h"
#include "gateway.h"


using namespace boost;
using boost::asio::ip::tcp;
using namespace std;

class session;
class client;

class server
{
private:
    asio::io_service io_service;
    std::vector<session*> *sessions;
    mysql_handler *mysql;
    tcp::acceptor *acceptor_;
    tcp::socket socket_;
    client *sm_client;
    gateway *gateway_;
public:
    bool test;
    server(short port, mysql_handler *_mysql, std::vector<session*> *_sessions, client *_client, gateway *_gateway);
            //: acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
            // socket_(io_service);
    void send_message(std::string node_id, std::string message);
    std::string send_message(std::string host, int port, std::string message);
    std::string send_message(std::string message);
    void do_accept();
};


