#pragma once
#include <string>
#include <cstring>
#include <memory>
#include <boost/asio.hpp>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <iostream>
#include <mysql_handler.h>
#include "../smartoffice-srv.h"
#include <server.h>
#include "response_handler.h"

class server;

using namespace boost;
using boost::asio::ip::tcp;
using namespace std;

class session : public std::enable_shared_from_this<session>
{

private:
    enum {
        max_length = 1024
    };

    tcp::socket socket_;

    char request_header[max_length];
    char request_body[max_length];
    std::vector<session*> *sessions;
    server *_server;

public:
    mysql_handler *mysql;
    std::string  *node_id = new std::string("");
    char transmitted_data_[max_length];
    char recieved_data_[max_length];
    void init(server *server);
    std::string get_node_id();
    session(tcp::socket socket, mysql_handler *_mysql, std::vector<session*> *sessions);
    ~session();
    void start();
    void send_message(std::string message);
private:
    void handle_request(size_t length);
    void do_read();
    void do_write(size_t length);
};

