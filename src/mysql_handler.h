#pragma once
#include <mysql++.h>
#include <string>
#include <cstdlib>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
using namespace std;
//using namespace boost;

class mysql_handler
{

private:
    mysqlpp::Connection *conn;
    vector<string> *hashes;
    vector<string> *attributes;
    string default_ip;
    short default_port;

public:
    bool connected = false;
    mysql_handler();
    ~mysql_handler();
    void connect(string database, string address, string user, string password);
    mysql_handler(string database, string address, string user, string password);
    std::pair<string, short> get_host_by_id(string node_id);
    void refresh_hashes();
    void refresh();
    std::pair<string, short> get_default_host();
    string get_attributes(string hash);
    void print_hashes();
    bool is_user_exists(string hash);
};

