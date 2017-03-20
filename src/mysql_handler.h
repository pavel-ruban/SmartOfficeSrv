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
public:
    string default_node_id;
private:
    string default_ip;
    short default_port;
    mysqlpp::Connection *conn;
    vector<string> *hashes;
    vector<string> *types;
    vector<string> *attributes;
public:
    bool connected = false;
    mysql_handler();
    ~mysql_handler();
    std::map<string, string> get_config();
    void connect(string database, string address, string user, string password);
    mysql_handler(string database, string address, string user, string password);
    std::pair<string, short> get_host_by_id(string node_id);
    void refresh_hashes();
    void refresh();
    std::pair<string, short> get_default_host();
    string get_attributes(string hash);
    string get_type(string hash);
    void print_hashes();
    bool is_user_exists(string hash);
};

