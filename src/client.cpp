#include <boost/thread.hpp>
#include "client.h"

client::client(std::vector<session*> *sessions, mysql_handler *mysql, gateway *_gateway) {
    gateway_ = _gateway;
    _sessions = sessions;
    _mysql = mysql;
    log_handler = new logger(cout, cerr);
    _mysql->refresh();
}

client::~client() {
    delete log_handler;
}

void client::my_read_until(boost::asio::ip::tcp::socket *_socket, boost::asio::streambuf *_response) {
    try {
        boost::asio::read_until(*_socket, *_response, "\n\n");
    } catch (std::exception &e)
    {

    }
}

void client::bo_auth() { //Костыльно.
    api_key = gateway_->get_config()->get_variable("API:auth:Api-key");
    string secret_key = gateway_->get_config()->get_variable("API:auth:Secret-key");
}

std::string client::send_message(std::string host, int port, std::string message, unsigned int _timeout, bool dnd) {
    boost::system::error_code ec;
    boost::asio::io_service ios;

    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);

    boost::asio::ip::tcp::socket _socket(ios);
    try {
        _socket.connect(endpoint);
        message = gateway_->magic(message);
        boost::array<char, 2048> buf;
        std::copy(message.begin(), message.end(), buf.begin());
        boost::system::error_code error;
        _socket.write_some(boost::asio::buffer(buf, message.size()), error);

        boost::asio::streambuf _response;
        log_handler->log_request(host + ":" + std::to_string(port), "", message, _timeout);
        if (_timeout) {
            boost::thread *newthread = new boost::thread(
                    boost::bind(&client::my_read_until, this, &_socket, &_response));
            if (!newthread->timed_join(boost::posix_time::seconds(_timeout / 1000))) {
                newthread->interrupt();
                if(_socket.is_open())
                {
                    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                    _socket.close();
                }
                throw std::logic_error("Client timed out.");
            }
            delete newthread;
        } else
            if (dnd)
            boost::asio::read_until(_socket, _response, "\0");
        _socket.close();
        std::string s( (std::istreambuf_iterator<char>(&_response)), std::istreambuf_iterator<char>() );
        log_handler->log_response(host + ":" + std::to_string(port),"", s);
        return gateway_->magic(s);

    } catch (std::exception &e) {
        throw;
    }
}

    string client::send_message(std::string _node_id, std::string message, unsigned int _timeout, bool dnd) {

        try {
            //vector<string> strs;
            //_node_id.erase(std::remove_if(_node_id.begin(),
            //                          _node_id.end(),
            //                          [](char x){return std::isspace(x);}),
            //           _node_id.end());
            //cout << _node_id << std::endl;
            //split(strs,_node_id,is_any_of(","));
            //cout << strs[0] << " " << strs[1] << endl;
            //for (auto vit = strs.begin(); vit != strs.end(); ++vit) {
               // cout << "DEST:" << _node_id << std::endl;
                for (auto it = _sessions->begin(); it != _sessions->end(); ++it) {
                    if ((*it)->get_node_id() == _node_id && (*it)->active) {
                       // cout << (*it)->get_node_id() << std::endl;

                        log_handler->log_request("", _node_id, message, _timeout);
                        if (_timeout)
                            (*it)->send_message(message, _timeout);
                        else
                            (*it)->send_message(message);
                        return "local_handle%";
                    }
                }
            //}

            return send_message(_mysql->get_host_by_id(_node_id).first, _mysql->get_host_by_id(_node_id).second, message, _timeout, dnd);
        } catch (std::exception &e) {
            throw;
        }
    }

    /**
     * Overloaded method, which picks default host from preselected MySQL database.
     *
     * @param message
     */
    std::string client::send_message(std::string message, unsigned int _timeout, bool dnd)
    {
        for ( std::string::iterator it=(message.end() - 3); it!=message.end();) {
            if (*it == '\n') {
                message.erase(it);
            } else
                it++;
        }
        message.append("\ndestination: " + _mysql->default_node_id + "\n\n");
       // log_handler->log_request((_mysql->get_default_host().first + ":" + std::to_string(_mysql->get_default_host().second)), "", message, _timeout);
        return send_message(_mysql->get_default_host().first, _mysql->get_default_host().second, message, _timeout, dnd);
    }

    void client::sam() {
        cout << _sessions->size() << std::endl;
    }