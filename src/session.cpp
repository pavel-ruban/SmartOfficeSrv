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
        active = true;
        log_handler->log("Client connected.");
        log_handler->clients_amount(sessions->size() + 1);
    }

    session::~session()
    {
        cout << "[LOG]: SESSION DC (node_id: " << this->get_node_id() << ")" << std::endl;
        for (auto it=timeout_messages.begin(); it!=timeout_messages.end(); ++it) {
            _client->send_message((*it).first, (*it).second, 0, false);
        }
        for (uint32_t i = 0; i < sessions->size(); i++) {
            if (this == (*sessions)[i]) {
                sessions->erase(sessions->begin() + i);
                break;
            }
        }
        delete log_handler;
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

void session::handle_error(string message, string dest, string origin, string action, bool log_this)
{
    if (dest == "") {
        dest = mysql->default_node_id;
    }
    if (message.find("Connection refused") != std::string::npos) {
        if (log_this)
            cout << "Connection refused (Server " << dest << " is down?)." << std::endl;
        _client->send_message(origin,
                              "status: 0\nnode_id: " + dest + "\ndestination: " + origin + "\naction: " + action +
                              "\n\n", 0, false);
    }
    if (message.find("Client timed out") != std::string::npos) {
        if (log_this)
            cout << "Client timed out (Server " << dest << " is down?)." << std::endl;
        _client->send_message(origin,
                              "status: 408\nnode_id: " + dest + "\ndestination: " + origin + "\naction: " + action +
                              "\n\n", 0, false);
    }
}

void session::disconnect(string reason)
{
    log_handler->log(get_node_id(), "Disconnected. Reason: " + reason);
    socket_.cancel();
}

void session::force_disconnect(string reason)
{
    char buf[max_length];
    char write_buf[2];
    write_buf[0] = '1';
    system::error_code ec;
    log_handler->log(get_node_id(), "Disconnected. Reason: " + reason);
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket_.cancel();

    auto self(shared_from_this());

    asio::async_write(
            socket_, asio::buffer(buf, std::strlen(buf)),
            [this, self](system::error_code ec, size_t length) {
                if (!ec) {
                    socket_.close();
                }
            }
    );
    send_message("BIBA");
}

    void session::handle_request(size_t length) {
        map<string,string> headers = parse_headers(string(recieved_data_));
        // Если запрос не содержит идентификатор.
        if (headers.find("node_id") == headers.end()) {
            log_handler->log(get_node_id(), "Request w/o auth info/bad request.");
            strcpy(transmitted_data_, "status: 400\n");
        } else {
            if (headers.find("node_id") != headers.end() && *node_id == "") {
                *node_id = (*headers.find("node_id")).second;
            } else {
                *node_id = "";
            }
            for (uint32_t i = 0; i < sessions->size(); i++) {
                if (this->get_node_id() == (*sessions)[i]->get_node_id() && this != (*sessions)[i]) {
                   // delete (*sessions)[i];
                   // (*sessions)[i]->force_disconnect("Old session with same node_id");
                    (*sessions)[i]->active = false;
                    break;
                }
            }

            mysql->refresh();
            string result("");
            if (mysql->is_user_exists(*node_id))
            {
                if (headers["action"] == "call")
                {
                    try {
                        if (headers.count("destination") <= 0) {
                            result = _client->send_message(string(recieved_data_), default_timeout, true);
                        } else {
                            result = _client->send_message(headers["destination"], string(recieved_data_), default_timeout, true);
                        }
                        handle_response(result);
                    } catch (std::exception &e){
                        if (headers.count("destination") > 0)
                            handle_error(string(e.what()), headers["destination"], headers["node_id"], headers["action"], true);
                        else
                            handle_error(string(e.what()), mysql->default_node_id, headers["node_id"], headers["action"], true);
                    }
                }
                if (headers["action"] == "open")
                {
                    try
                    {
                        mysql->refresh();
                        cout << mysql->get_type(*node_id);
                        if (mysql->get_type(*node_id) == "bo") {
                            result = _client->send_message(headers["destination"], string(recieved_data_), default_timeout, true);
                            if (result != "local_handle%")
                                handle_response(result);
                        }
                        if (mysql->get_type(*node_id) == "pcd") {
                            result = _client->send_message(headers["destination"], string(recieved_data_), 0, true);
                        }
                    } catch (std::exception &e)
                    {
                        handle_error(string(e.what()), headers["destination"], headers["node_id"], headers["action"], true);
                    }
                }
                if (headers["action"] == "access request")
                {
                    try {
                        mysql->refresh();
                        if (mysql->get_type(*node_id) == "pcd") {
                            if (headers.count("destination") <= 0) {
                                result = _client->send_message(string(recieved_data_), default_timeout, true);
                            } else {
                                result = _client->send_message(headers["destination"], string(recieved_data_),
                                                               default_timeout, true);
                            }
                            if (result != "local_handle%")
                                handle_response(result);
                        }
                    } catch (std::exception &e) {
                        handle_error(string(e.what()), headers["destination"], headers["node_id"], headers["action"], true);
                    }
                }
            } else {
                log_handler->log(get_node_id(), "Request auth info does not match any legit clients.");
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
                    bool call_active = false;
                    string _response("");
                    try {
                        _response = _client->send_message(headers["destination"], string("node_id: ") + headers["node_id"] +
                                                                                  string("\naction: call\ndestination: " +
                                                                                         headers["destination"]), default_timeout, true);
                        call_active = true;
                        try {
                            auto bufheaders = parse_headers(_response);
                            _client->send_message(bufheaders["destination"],
                                                  "status: " + bufheaders["status"] + "\nnode_id: " +
                                                  bufheaders["node_id"]
                                                  + "\naction: " + bufheaders["action"] + "\ndestination: " +
                                                  bufheaders["destination"] + "\n\n", 0, false);
                            _client->send_message(headers["node_id"],
                                                  "status: " + bufheaders["status"] + "\nnode_id: " +
                                                  bufheaders["node_id"]
                                                  + "\naction: " + bufheaders["action"] + "\ndestination: " +
                                                          headers["node_id"] + "\n\n", 0, false);
                        } catch (std::exception &e) {

                        }
                    } catch (std::exception &e) {
                        handle_error(string(e.what()), headers["destination"], headers["node_id"], headers["action"], true);
                        handle_error(string(e.what()), headers["destination"], get_node_id(), headers["action"], false);
//                        _client->send_message(headers["node_id"],
//                                              "status: 0\nnode_id: " +
//                                              headers["destination"]
//                                              + "\naction: " + headers["action"] + "\ndestination: " +
//                                              headers["node_id"] + "\n\n", 0);
                    }
                }
                if (headers["action"] == "open")
                {
                    try {
                        _client->send_message(headers["destination"], response, 0, false);
                    } catch (std::exception &e) {
                        handle_error(string(e.what()), headers["destination"], headers["node_id"], headers["action"], true);
                    }
                }
                if (headers["action"] == "access request")
                {
                    try {
                       // usleep(200);
                        _client->send_message(headers["destination"], response, 0, false);
                    } catch (std::exception &e) {
                        handle_error(string(e.what()), headers["destination"], headers["node_id"], headers["action"], true);
                    }
                }
            } else {
                _client->send_message(*node_id, "status: server error\n", 0, false);
                return;
            }
        } else {
            _client->send_message(*node_id, "status: server error\n", 0, false);
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
                        log_handler->log_response(socket_.remote_endpoint().address().to_string(),get_node_id(), recieved_data_);
                        struct timeval tp;
                        gettimeofday(&tp, NULL);
                        mslong = 0;
                        _timeout = 0;
                        handle_request(length);
                        do_write(length);
                    } else {
                        std::cerr << "NA READE: ";
                        std::cerr << ec << std::endl;
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
        //sleep(1);
        auto self(shared_from_this());
        asio::async_write(
                socket_, asio::buffer(message.c_str(), message.length() + 2),
                [this, self](system::error_code ec, size_t length) {
                    if (!ec) {
                       // do_read();
                    } else {
                        std::cerr << "NA SENDE: ";
                        std::cerr << ec << std::endl;
                    }
                }
        );
    }

void session::send_message(std::string message, unsigned int timeout) {
    auto headers = parse_headers(message);
    timeout_messages.insert(std::make_pair(headers["node_id"], "node_id: " + get_node_id() +"\nstatus: 408\naction: " + headers["action"] + "\n\n"));
    _timeout = timeout;
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
}
