#include <boost/thread.hpp>
#include "client.h"

client::client(std::vector<session*> *sessions, mysql_handler *mysql) {
    _sessions = sessions;
    _mysql = mysql;
}

void client::my_read_until(boost::asio::ip::tcp::socket *_socket, boost::asio::streambuf *_response) {
    boost::asio::read_until(*_socket, *_response, "\n\n");
}

std::string client::send_message(std::string host, int port, std::string message) {
    boost::system::error_code ec;
    boost::asio::io_service ios;

    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);

    boost::asio::ip::tcp::socket _socket(ios);
    try {
        _socket.connect(endpoint);
        boost::array<char, 2048> buf;
        std::copy(message.begin(), message.end(), buf.begin());
        boost::system::error_code error;
        _socket.write_some(boost::asio::buffer(buf, message.size()), error);

        boost::asio::streambuf _response;
        boost::thread *newthread = new boost::thread(boost::bind(&client::my_read_until, this, &_socket, &_response));
        if (!newthread->timed_join(boost::posix_time::seconds(5))) {
            newthread->interrupt();
            //cout << "SHIT HAPPENS" << std::endl;
            throw std::logic_error("Client timed out.");
        }
        delete newthread;
       // boost::asio::read_until(_socket, _response, "\n\n");
        _socket.close();
        std::string s( (std::istreambuf_iterator<char>(&_response)), std::istreambuf_iterator<char>() );
        cout << "Answer from " << host << ":" << port << " is:\n" << s << "----------------------------------------" << std::endl;
        return s;

    } catch (std::exception &e) {
        //cerr << e.what() << std::endl;
        throw;
    }
}

    string client::send_message(std::string _node_id, std::string message) {
        try {
            for (auto it = _sessions->begin(); it != _sessions->end(); ++it) {
                if ((*it)->get_node_id() == _node_id) {
                    (*it)->send_message(message);
                    return "local_handle%";
                }
            }

            return send_message(_mysql->get_host_by_id(_node_id).first, _mysql->get_host_by_id(_node_id).second, message);
        } catch (std::exception &e) {
            throw;
        }
    }

    /**
     * Overloaded method, which picks default host from preselected MySQL database.
     *
     * @param message
     */
    std::string client::send_message(std::string message)
    {
        cout << "Default web server info: " << _mysql->get_default_host().first << " " << _mysql->get_default_host().second << std::endl;
        return send_message(_mysql->get_default_host().first, _mysql->get_default_host().second, message);
    }

    void client::sam() {
        cout << _sessions->size() << std::endl;
    }