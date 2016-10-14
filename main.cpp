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
#include <mysql++.h>
#include <boost/array.hpp>
#include <boost/thread.hpp>

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
    string ip;
    session(tcp::socket socket) : socket_(std::move(socket)) {
        memset(transmitted_data_, 0, sizeof(transmitted_data_));
        memset(recieved_data_, 0, sizeof(recieved_data_));
    }

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
                    cout << "Recieved from: " << socket_.remote_endpoint().address().to_string() << endl;
                    cout << recieved_data_ << endl;
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
            socket_, asio::buffer(transmitted_data_, std::strlen(transmitted_data_)),
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

};

class server
{
public:
    server(asio::io_service& io_service, short port)
            : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
              socket_(io_service) {
        do_accept();
    }

    void send_message(std::string host, int port, std::string message) {
        boost::system::error_code ec;
        boost::asio::io_service ios;

        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);

        boost::asio::ip::tcp::socket socket(ios);

        socket.connect(endpoint);

        boost::array<char, 2048> buf;
        std::copy(message.begin(),message.end(),buf.begin());
        boost::system::error_code error;
        socket.write_some(boost::asio::buffer(buf, message.size()), error);

        std::string response;

        do {
            char buf[1024];
            size_t bytes_transferred = socket.receive(asio::buffer(buf, 1024), {}, ec);
            if (!ec) response.append(buf, buf + bytes_transferred);
        } while (!ec);
        socket.close();
    }

private:
    void do_accept() {
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

void start_server(short port, server * _server) {
    asio::io_service io_service;
    server s(io_service, port);
    _server = &s;
    io_service.run();
}

int main(int argc, char* argv[])
{
    server* server;
    mysqlpp::Connection conn(false);
    conn.set_option(new mysqlpp :: SetCharsetNameOption ("utf8"));
    conn.connect("smartoffice_srv", "localhost", "somi", "somi2016");

    try {
        short port;

        property_tree::ptree pt;
        property_tree::ini_parser::read_ini("config.ini", pt);
        istringstream (pt.get<string>("General.port")) >> port;
        cout << port << std::endl;

        boost::thread{start_server, port, server};

        char chars[20];
        while (true) {
            scanf("%s",chars);
            server->send_message("192.168.1.113", 2222, chars);
        }
    }
    catch (std::exception &e) {
        cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}