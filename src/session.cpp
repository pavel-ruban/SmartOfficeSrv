#include "session.h"
#include <memory>

    std::string session::get_node_id()
    {
        if (*node_id != "") {
            return *node_id;
        } else return "Unathorized";
    }

    session::session(tcp::socket socket, mysql_handler *_mysql, std::vector<session*> *_sessions) : socket_(std::move(socket))
    {
        sessions = _sessions;
        mysql = _mysql;
        memset(transmitted_data_, 0, sizeof(transmitted_data_));
        memset(recieved_data_, 0, sizeof(recieved_data_));
        *node_id = "";
        sessions = new std::vector<session*>();

    }

    session::~session()
    {
        cout << "SESSION DC (node_id: " << this->get_node_id() << ")" << std::endl;
        for (uint32_t i = 0; i < sessions->size(); i++) {
            if (this == (*sessions)[i]) {
                sessions->erase(sessions->begin() + i);
                break;
            }
        }
        delete node_id;
    }

    void session::init(server *server)
    {
        _server = server;
    }

    void session::start()
    {
        do_read();
    }
///////

    void session::handle_request(size_t length) {

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

        while (regex_search(start, end, what, expression, flags))
        {
            start = what[0].second;
            string header(what[1].first, what[1].second);
            headers_s = header;
            string body(what[2].first, what[2].second);
        }

        strs.clear();
        split(strs,headers_s,is_any_of("\n"));

        strs.erase(strs.end() - 1);

        for(vector<string>::iterator it = strs.begin(); it != strs.end(); ++it)
        {
            buf.clear();
            split(buf,*it,is_any_of(":"));
            headers[buf[0]] = buf[1].substr(1,buf[1].size());
        }

        // Если запрос не содержит идентификатор.
        if (headers.find("node_id") == headers.end()) {
            strcpy(transmitted_data_, "status: 400\n");
        } else {
            if (headers.find("node_id") != headers.end()) {
                *node_id = (*headers.find("node_id")).second;
            } else
                *node_id = "";

            mysql->refresh_hashes();
            if (mysql->is_user_exists(*node_id)) {
                if (headers["action"] == "call")
                    if(headers.find(headers["destination"]) == headers.end()) {
                        _server->send_message(string(transmitted_data_));
                    } else {
                        _server->send_message(headers["destination"], string(transmitted_data_));
                    }
                // Звонок.
                cout << "RING" << endl;
                strcpy(transmitted_data_, "status: 200\n");
            } else
                strcpy(transmitted_data_, "status: 401\n");

        }
    }

    void session::do_read()
    {
        auto self(shared_from_this());
        socket_.async_read_some(
                asio::buffer(recieved_data_, max_length),
                [this, self](system::error_code ec, size_t length)
                {
                    if (!ec) {
                        cout << "Recieved from: " << socket_.remote_endpoint().address().to_string() << endl;
                        cout << recieved_data_ << endl;
                        handle_request(length);
                        do_write(length);
                    }
                }
        );
    }

    void session::do_write(size_t length)
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

    void session::send_message(std::string message) {
        strcpy(transmitted_data_, message.c_str());
        do_write(message.length());
    }
