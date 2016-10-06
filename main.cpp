#include <iostream>
#include "smartoffice-srv.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>

using boost::asio::ip::tcp;
using namespace std;
using namespace boost;

enum {
    OK = 200,
    FORBIDDEN = 403,
    UNAUTHORIZED = 401,
    INTERNAL_SERVER_ERROR = 500
};

class session
        : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start()
    {
        do_read();
    }

private:
    enum {
        max_length = 1024
    };

    char request_header[max_length];
    char request_body[max_length];

    void handle_request(size_t length) {
        vector<string> strs;
        vector<string> buf;

        string headers_s;

        map<string,string> headers;

        typedef map<string, int, less<string> > map_type;
        regex expression("^(.+(?:\r\n|\n|\r))+(?:\r\n|\n|\r)(.*)$");

        string::const_iterator start, end;
        string data(recieved_data_);

        start = data.begin();
        end = data.begin() + length;

        match_results<string::const_iterator> what;
        match_flag_type flags = regex_constants::match_single_line | regex_constants::match_stop;

        while (regex_search(start, end, what, expression, flags)) {
            start = what[0].second;
            string header(what[1].first, what[1].second);
            headers_s = header;
            string body(what[2].first, what[2].second);
        }

        strs.clear();
        split(strs,headers_s,is_any_of("\n"));

        strs.erase(strs.end() - 1);

        for(vector<string>::iterator it = strs.begin(); it != strs.end(); ++it) {
            buf.clear();
            split(buf,*it,is_any_of(":"));
            headers[buf[0]] = buf[1].substr(1,buf[1].size());
        }

        // Если запрос не содержит идентификатор.
        if (headers.find("node_id") == headers.end()) {
            strcpy(transmitted_data_, "status: 401\n");
        } else {
            // Тут sql авторизация?
            if (headers["action"] == "call");

            // Звонок.
            cout << "RING" << endl;
            strcpy(transmitted_data_, "status: 200\n");

        }
    }

    void do_read()
    {
        auto self(shared_from_this());
        socket_.async_read_some(
            asio::buffer(recieved_data_, max_length),
            [this, self](system::error_code ec, size_t length) {
                if (!ec) {
                    handle_request(length);
                    do_write(length);
                }
            }
        );
    }

    void do_write(size_t length)
    {
        auto self(shared_from_this());
        asio::async_write(
            socket_, asio::buffer(transmitted_data_, max_length),
            [this, self](system::error_code ec, size_t length) {
                 if (!ec) {
                     do_read();
                 }
            }
        );
    }

    tcp::socket socket_;

    char transmitted_data_[max_length];
    char recieved_data_[max_length];

public:
    void send_request(std::string request_string) {
       // do
    }
};

class server
{
public:
    server(asio::io_service& io_service, short port)
            : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
              socket_(io_service) {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            socket_,
            [this](system::error_code ec) {
                if (!ec) {
                    std::make_shared<session>(std::move(socket_))->start();
                }

                do_accept();
            }
        );
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
};

int main(int argc, char* argv[])
{

    try {
        short port;

        property_tree::ptree pt;
        property_tree::ini_parser::read_ini("config.ini", pt);
        asio::io_service io_service;

        istringstream (pt.get<string>("General.port")) >> port;

        server s(io_service, port);

        io_service.run();
    }
    catch (std::exception &e) {
        cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}