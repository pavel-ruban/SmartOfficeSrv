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
#include <chrono>
#include "response_handler.h"
#include "client.h"
#include "logger.h"
#include <gateway.h>
#include <boost/asio/read.hpp>
#include <boost/asio/placeholders.hpp>

class client;

using namespace boost;
using boost::asio::ip::tcp;
using namespace std;
using namespace std::chrono;

class session : public std::enable_shared_from_this<session>
{

private:
    enum {
        max_length = 1024,
        default_timeout = 10000
    };
    logger *log_handler = new logger(cout, cout);
    char request_header[max_length];
    char request_body[max_length];
    std::vector<session*> *sessions;
    client *_client;
    gateway *_gateway;

public:
    //
    std::map<int,string> *test = new std::map<int, string>;
    //
    bool active;
    void force_disconnect(string reason);
    void disconnect(string reason);
    void send_message(std::string message, unsigned int timeout);
    void refresh_time();
    long long mslong = 0;
    unsigned int _timeout = 0;
    mysql_handler *mysql;
    std::string  *node_id = new std::string("");
    char transmitted_data_[max_length];
    char recieved_data_[max_length];
    std::string get_node_id();
    session(tcp::socket socket, mysql_handler *_mysql, std::vector<session*> *sessions, client *_client, gateway *_gateway);
    ~session();
    void start();
    void send_message(std::string message);
    map<string, string> parse_headers(string data_to_parse);
    void handle_error(string message, string dest, string origin, string action, bool log_this);

private:

    std::multimap<string, string> timeout_messages;
    tcp::socket socket_;
    void handle_response(string response);
    void handle_request(size_t length);
    void handle_read(const boost::system::error_code& err,
                              std::size_t bytes_transferred);
    void do_read();
    void do_write(size_t length);
};

