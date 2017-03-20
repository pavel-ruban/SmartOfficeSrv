#include "mysql_handler.h"
#include "smartoffice-srv.h"
#include <boost/property_tree/ini_parser.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include "mysql_handler.h"
#include "session.h"
#include "server.h"
#include "client.h"
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
	s = new server(port, mysql, sessions, sm_client, &_gateway);
}

void invalidate_sessions()
{
	while (true) {
		struct timeval tp;
		gettimeofday(&tp, NULL);
		usleep(900000);
		for (uint32_t i = 0; i < sessions->size(); i++) {
			if ((*sessions)[i]->_timeout) {
				if ((((*sessions)[i]->mslong + (*sessions)[i]->_timeout)
					<=	(long long) (tp.tv_sec * 1000L + tp.tv_usec / 1000)) && (*sessions)[i]->mslong != 0) {

					(*sessions)[i]->_timeout = 0;
					(*sessions)[i]->disconnect("Connection timed out.");
				}
			}
		}
	}
}

int main(int argc, char* argv[])
{
	short port = _config->get_port();
	cout << port << std::endl;

	try {
		boost::thread{*invalidate_sessions};
		boost::thread{*start_server, port};
		char chars[20];
		while (true) {
			scanf("%s",chars);

			switch (chars[0]) {
				case 'a':
					sm_client->send_message("lock_test", "ZDAROVA\n", 10000, true);
					break;
				case 'b':
					//cout << sessions->size();
					sm_client->sam();
					break;
				case 'c':
					cout << (*sessions)[0]->get_node_id() << std::endl;
					cout << (*sessions)[1]->get_node_id() << std::endl;
					break;
				case 'd':
					cout << mysql->get_attributes("237c8e4d3d631e604fsdf23") << std::endl;
					break;
			}
		}
	}
	catch (std::exception &e) {
		cerr << "Exception: " << e.what() << "\n";
	}

	delete mysql;
	return 0;
}

