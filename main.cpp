#include "mysql_handler.h"
#include "smartoffice-srv.h"
#include <boost/property_tree/ini_parser.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include "mysql_handler.h"
#include "session.h"
#include "server.h"
#include "client.h"
#include "client_asio.h"
#include "gateway.h"
#include <config.h>

using namespace std;

class server;
server *s;
client *sm_client;
mysql_handler *mysql = new mysql_handler();
config *_config = new config("config.ini", *mysql);
gateway _gateway(mysql, _config);
std::vector<session*> *sessions = new std::vector<session*>();

asio::io_service io_service;

void start_server(short port)
{
    sm_client = new client(sessions, mysql, &_gateway);
    s = new server(io_service, port, mysql, sessions, sm_client, &_gateway);
    io_service.run();
}

void invalidate_sessions()
{
    while (true) {
        struct timeval tp;
        gettimeofday(&tp, NULL);
        usleep(900000);
        for (uint32_t i = 0; i < sessions->size(); i++) {
            if ((*sessions)[i]->_timeout) {
                if ((((*sessions)[i]->mslong + (*sessions)[i]->_timeout) <=
                     (long long) (tp.tv_sec * 1000L + tp.tv_usec / 1000)) && (*sessions)[i]->mslong != 0) {
                    (*sessions)[i]->_timeout = 0;
                    (*sessions)[i]->disconnect("Connection timed out.");
                }
            }
        }
    }
}

int main(int argc, char* argv[])
{
//    boost::asio::io_service _io_service;
//        tcp::resolver _r(_io_service);
//        client_asio c(_io_service);
//
//        c.start(_r.resolve(tcp::resolver::query("192.168.1.209", "3344")));
    short port = _config->get_port();
//    std::string database, server_address, user, password;
//    property_tree::ptree pt;
//    property_tree::ini_parser::read_ini("config.ini", pt);
//    istringstream (pt.get<string>("General.port")) >> port;
//    istringstream (pt.get<string>("MySQL.database")) >> database;
//    istringstream (pt.get<string>("MySQL.address")) >> server_address;
//    istringstream (pt.get<string>("MySQL.user")) >> user;
//    istringstream (pt.get<string>("MySQL.password")) >> password;
    cout << port << std::endl;
//    mysql->connect(database, server_address, user, password);
//    mysql->refresh();
   // mysql->print_hashes();
    //native_to_http biba("node_id: biba\naction: access request\ndestination: bo_test\n\n\n", _config);
    //std::string test = biba.convert();
    //string test = _gateway.magic("node-id: biba\naction: access request\ndestination: bo_test\n\n\n");
    //string test1 = _gateway.magic("node-id: biba\naction: access request\ndestination: node_test\n\n\n");
//    _gateway.convert("");
    try {
        boost::thread{*invalidate_sessions};
        boost::thread{*start_server, port};
        char chars[20];
        while (true) {
            scanf("%s",chars);
            if(chars[0] == 'a')
               sm_client->send_message("lock_test", "ZDAROVA\n", 10000, true);
            int i = 0;
            if(chars[0] == 'b') {
                //cout << sessions->size();
                sm_client->sam();
            }
            if(chars[0] == 'c')
                cout << (*sessions)[0]->get_node_id() << std::endl;
            cout << (*sessions)[1]->get_node_id() << std::endl;
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