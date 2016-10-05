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
    session(tcp::socket socket)
            : socket_(std::move(socket))
    {
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

    void handle_request(std::size_t length) {
        std::vector<std::string> strs;
        std::vector<std::string> buf;
        std::string headers_s;
        std::map<std::string,std::string> headers;

        typedef std::map<std::string, int, std::less<std::string> > map_type;
        boost::regex expression("^(.+(?:\r\n|\n|\r))+(?:\r\n|\n|\r)(.*)$");

        std::string::const_iterator start, end;
        std::string data(recieved_data_);

        start = data.begin();
        end = data.begin() + length;

        boost::match_results<std::string::const_iterator> what;
        boost::match_flag_type flags = boost::regex_constants::match_single_line | boost::regex_constants::match_stop;

        while (regex_search(start, end, what, expression, flags)) {
            start = what[0].second;
            std::string header(what[1].first, what[1].second);
            headers_s = header;
            std::string body(what[2].first, what[2].second);
            int x = 1;
        }

        strs.clear();
        boost::split(strs,headers_s,boost::is_any_of("\n"));

        strs.erase(strs.end() - 1);
        for(std::vector<std::string>::iterator it = strs.begin(); it != strs.end(); ++it) {
            buf.clear();
            boost::split(buf,*it,boost::is_any_of(":"));
            headers[buf[0]] = buf[1].substr(1,buf[1].size());
        }

        if (headers.find("node_id") == headers.end()) {  //Если запрос не содержит идентификатор.
            std::strcpy(transmitted_data_, "status: 401\n");
        } else {
            //Тут sql авторизация?
            if (headers["action"] == "call");
            std::cout << "RING" << std::endl; //Ну типа звонок.
            std::strcpy(transmitted_data_, "status: 200\n");

        }
        int x = 100500;


    }
    void do_read()
    {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(recieved_data_, max_length),
                                [this, self](boost::system::error_code ec, std::size_t length) {
                                    if (!ec) {
                                        handle_request(length);
                                        do_write(length);
                                    }
                                });
    }

    void do_write(std::size_t length)
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(transmitted_data_, max_length),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                                     if (!ec) {
                                         do_read();
                                     }
                                 });
    }

    tcp::socket socket_;

    char transmitted_data_[max_length];
    char recieved_data_[max_length];
};

class server
{
public:
    server(boost::asio::io_service& io_service, short port)
            : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
              socket_(io_service) {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(socket_,
                               [this](boost::system::error_code ec) {
                                   if (!ec) {
                                       std::make_shared<session>(std::move(socket_))->start();
                                   }

                                   do_accept();
                               });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
};

int main(int argc, char* argv[])
{

    try {
        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini("../include/config", pt);
        boost::asio::io_service io_service;
        short port;
        std::istringstream (pt.get<std::string>("General.port")) >> port;

        server s(io_service, port);

        io_service.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}