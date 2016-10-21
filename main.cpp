#include "mysql_handler.h"
#include "smartoffice-srv.h"
#include <boost/property_tree/ini_parser.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include "mysql_handler.h"
#include "session.h"
#include "server.h"

using namespace std;

class server;
server *s;

mysql_handler *mysql = new mysql_handler();
std::vector<session*> *sessions = new std::vector<session*>();

asio::io_service io_service;

void start_server(short port)
{
    s = new server(io_service, port, mysql, sessions);
    io_service.run();
}

int main(int argc, char* argv[])
{
    short port;
    std::string database, server_address, user, password;
    property_tree::ptree pt;
    property_tree::ini_parser::read_ini("config.ini", pt);
    istringstream (pt.get<string>("General.port")) >> port;
    istringstream (pt.get<string>("MySQL.database")) >> database;
    istringstream (pt.get<string>("MySQL.address")) >> server_address;
    istringstream (pt.get<string>("MySQL.user")) >> user;
    istringstream (pt.get<string>("MySQL.password")) >> password;
    cout << port << std::endl;
    mysql->connect(database, server_address, user, password);
    mysql->refresh_hashes();
    mysql->refresh();
    mysql->print_hashes();
    try {
        boost::thread{*start_server, port};
        char chars[20];
        while (true) {
            scanf("%s",chars);
            if(chars[0] == 'a')
            s->send_message("ZDAROVA");
            int i = 0;
            if(chars[0] == 'b')
            cout << sessions->size();
            if(chars[0] == 'c')
                cout << (*sessions)[0]->get_node_id() << std::endl;
            if(chars[0] == 'd')
                cout << mysql->get_attributes("237c8e4d3d631e604fsdf23") << std::endl;
        }
    }
    catch (std::exception &e) {
        cerr << "Exception: " << e.what() << "\n";
    }

    delete mysql;
    return 0;
}