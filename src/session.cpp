#include "session.h"
#include <memory>

std::string session::get_node_id()
    {
        if (*node_id != "") {
            return *node_id;
        } else return "Unathorized";
    }

    session::session(tcp::socket socket, mysql_handler *_mysql, std::vector<session*> *_sessions, client *sclient) : socket_(std::move(socket))
    {
        sessions = _sessions;
        mysql = _mysql;
        _client = sclient;
        memset(transmitted_data_, 0, sizeof(transmitted_data_));
        memset(recieved_data_, 0, sizeof(recieved_data_));
        *node_id = "";
        //sessions = new std::vector<session*>();

    }

    session::~session()
    {
        cout << "[LOG]: SESSION DC (node_id: " << this->get_node_id() << ")" << std::endl;
        for (uint32_t i = 0; i < sessions->size(); i++) {
            if (this == (*sessions)[i]) {
                sessions->erase(sessions->begin() + i);
                break;
            }
        }
        delete node_id;
    }

    map<string, string> session::parse_headers(string data_to_parse) {
        vector<string> strs;
        vector<string> buf;

        string headers_s;

        map<string,string> headers;

        typedef map<string, int, less<string> > map_type;
        regex expression("^(.+(?:\r\n|\n|\r))+(?:\r\n|\n|\r)(.*)$");

        string::const_iterator start, end;

        start = data_to_parse.begin();
        end = data_to_parse.begin() + data_to_parse.length();

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
            if (buf[0] != "" && buf.size() > 1)
                headers[buf[0]] = buf[1].substr(1,buf[1].size());
        }
        return headers;
    }

    void session::start()
    {
        do_read();
    }
///////
void session::handle_error(string message, string dest, string origin, string action)
{
    if (message.find("Connection refused") != std::string::npos) {
        cout << "Connection refused (Server " << dest << " is down?)." << std::endl;
        _client->send_message(*node_id,
                              "status: 0\nnode_id: " + dest + "\ndestination: " + *node_id + "\naction: " + action +
                              "\n\n");
    }
    if (message.find("Client timed out") != std::string::npos) {
        cout << "Client timed out (Server " << dest << " is down?)." << std::endl;
        _client->send_message(*node_id,
                              "status: 408\nnode_id: " + dest + "\ndestination: " + *node_id + "\naction: " + action +
                              "\n\n");
    }
}

    void session::handle_request(size_t length) {
        map<string,string> headers = parse_headers(string(recieved_data_));
        // Если запрос не содержит идентификатор.
        if (headers.find("node_id") == headers.end()) {
            cout << "[LOG]: Request w/o auth info/bad request." << std::endl;
            strcpy(transmitted_data_, "status: 400\n");
        } else {
            if (headers.find("node_id") != headers.end() && *node_id == "") {
                *node_id = (*headers.find("node_id")).second;
            } else {
                *node_id = "";
            }

            mysql->refresh_hashes();
            string result("");
            if (mysql->is_user_exists(*node_id))
            {
                if (headers["action"] == "call")
                {
                    try {
                        cout << "----------------------------------------\nTrying to send: destination is ";
                        if (headers.count("destination") <= 0) {
                            cout << "default web server." << "\n" << recieved_data_ <<  std::endl;
                            cout << "----------------------------------------" << std::endl;
                            result = _client->send_message(string(recieved_data_));

                        } else {
                            cout << headers["destination"] << "\n" << recieved_data_ << std::endl;
                            cout << "----------------------------------------" << std::endl;
                            result = _client->send_message(headers["destination"], string(recieved_data_));
                        }
                        handle_response(result);
                    } catch (std::exception &e){
                            handle_error(string(e.what()), headers["destination"], headers["node_id"], headers["action"]);
                    }
                }
                if (headers["action"] == "open")
                {
                    try
                    {
                        //cout << "----------------------------------------\nTrying to send: destination is ";
                        mysql->refresh();
                        if (mysql->get_type(*node_id) == "bo") {
                            result = _client->send_message(headers["destination"], string(recieved_data_));
                            if (result != "local_handle%")
                                handle_response(result);
                        }
                        if (mysql->get_type(*node_id) == "pcd") {
                            result = _client->send_message(headers["destination"], string(recieved_data_));
                        }
                    } catch (std::exception &e)
                    {
                        handle_error(string(e.what()), headers["destination"], headers["node_id"], headers["action"]);
                    }
                }
                if (headers["action"] == "access request")
                {
                    try {
                        mysql->refresh();
                        if (mysql->get_type(*node_id) == "pcd") {
                            result = _client->send_message(headers["destination"], string(recieved_data_));
                            if (result != "local_handle%")
                                handle_response(result);
                        }
                    } catch (std::exception &e) {
                        handle_error(string(e.what()), headers["destination"], headers["node_id"], headers["action"]);
                    }
                }
            } else {
                cout << "[LOG]: Request auth info does not match any legit clients." << std::endl;
                strcpy(transmitted_data_, "status: 401\n");
            }

        }
    }

    void session::handle_response(string response) {
        map<string,string> headers = parse_headers(response);
        if(headers.count("node_id") > 0)
        {
            if (mysql->is_user_exists(headers["node_id"]))
            {
                if (headers["action"] == "call")
                {
                    string _response("");
                    cout << "----------------------------------------\nTrying to send: destination is " << headers["destination"] << std::endl;
                    cout << string("node_id: ") + *node_id + string("\naction: call \ndestination: " + headers["destination"]) << std::endl;
                    cout << "----------------------------------------" << std::endl;
                    try {
                        _response = _client->send_message(headers["destination"], string("node_id: ") + *node_id +
                                                                                  string("\naction: call\ndestination: " +
                                                                                         headers["destination"]));
                        auto bufheaders = parse_headers(_response);
                        cout << _client->send_message(bufheaders["destination"],"status: " +bufheaders["status"] + "\nnode_id: " + bufheaders["node_id"] + "\naction: " + bufheaders["action"] + "\n") << std::endl;
                    } catch (std::exception &e) {
                        handle_error(string(e.what()), headers["destination"], headers["node_id"], headers["action"]);
                    }
                }
                if (headers["action"] == "open")
                {
                    try {
                        _client->send_message(headers["destination"], response);
                    } catch (std::exception &e) {
                        handle_error(string(e.what()), headers["destination"], headers["node_id"], headers["action"]);
                    }
                }
                if (headers["action"] == "access request")
                {
                    try {
                        _client->send_message(headers["destination"], response);
                    } catch (std::exception &e) {
                        handle_error(string(e.what()), headers["destination"], headers["node_id"], headers["action"]);
                    }
                }
            } else {
                _client->send_message(*node_id, "status: server error\n");
                return;
            }
        } else {
            _client->send_message(*node_id, "status: server error\n");
            return;
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
                        cout << "----------------------------------------\nRecieved from: " << socket_.remote_endpoint().address().to_string() << endl;
                        cout << recieved_data_ << "----------------------------------------" << endl;
                        struct timeval tp;
                        gettimeofday(&tp, NULL);
                        mslong = 0;
                        //cout << ((long long) tp.tv_sec * 1000L + tp.tv_usec / 1000) - mslong << std::endl;
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

    void session::refresh_time() {
        struct timeval tp;
        gettimeofday(&tp, NULL);
        mslong = (long long) tp.tv_sec * 1000L + tp.tv_usec / 1000;
    }


    void session::send_message(std::string message) {
        refresh_time();
        auto self(shared_from_this());
        asio::async_write(
                socket_, asio::buffer(message.c_str(), message.length()),
                [this, self](system::error_code ec, size_t length) {
                    if (!ec) {
                       // do_read();
                    }
                }
        );
//        memset(transmitted_data_, 0, sizeof(transmitted_data_));
//        strcpy(transmitted_data_, message.c_str());
//        avaiting_answer = true;
//        do_write(message.length());
    }
