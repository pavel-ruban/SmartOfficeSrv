#pragma once
#include <string>
#include <cstring>
#include <memory>
#include <boost/asio.hpp>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <iostream>
#include <boost/array.hpp>
#include "session.h"
#include "mysql_handler.h"
#include "logger.h"

using namespace boost;
using boost::asio::ip::tcp;
using namespace std;

class session;

class client {
private:
    std::vector<session *> *_sessions;
    mysql_handler *_mysql;
    logger *log_handler;
public:
    void sam();
    void my_read_until(boost::asio::ip::tcp::socket *_socket, boost::asio::streambuf *_response);
    client(std::vector<session *> *sessions, mysql_handler *mysql);
    ~client();
    std::string send_message(std::string host, int port, std::string message, unsigned int _timeout, bool dnd);
    string send_message(std::string node_id, std::string message, unsigned int _timeout, bool dnd);
    std::string send_message(std::string message, unsigned int _timeout, bool dnd);
};

