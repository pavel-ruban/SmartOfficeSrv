#include "session.h"
#include "server.h"



    server::server(asio::io_service& io_service, short port, mysql_handler *_mysql, std::vector<session*> *_sessions)
            : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
              socket_(io_service)
    {
        sessions = _sessions;
        mysql = _mysql;
        do_accept();
    }

    void server::send_message(std::string node_id, std::string message) {
        for (auto it = sessions->begin(); it != sessions->end(); ++it) {
            if ((*it)->get_node_id() == node_id) {
                (*it)->send_message(message);
            }
        }
    }

    std::string server::send_message(std::string host, int port, std::string message) {
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

            std::string response;
            do {
                char buf[2048];
                size_t bytes_transferred = _socket.receive(asio::buffer(buf, 2048), {}, ec);
                if (!ec) {
                    response.append(buf, buf + bytes_transferred);
                }
            } while (!ec && response.length() <= 0);
            _socket.close();
            return response;

        } catch (std::exception &e) {
            cerr << e.what() << std::endl;
        }
    }


    /**
     * Overloaded method, which picks default host from preselected MySQL database.
     *
     * @param message
     */
    std::string server::send_message(std::string message)
    {
        //cout << mysql->get_default_host().first;
        cout << mysql->get_default_host().first << " " << mysql->get_default_host().second << std::endl;
        return send_message(mysql->get_default_host().first, mysql->get_default_host().second, message);
    }

    char error_data[1000];
    std::size_t error_size;
    void server::do_accept()
    {
        acceptor_.async_accept(
                socket_,
                [this](system::error_code ec) {
                    std::shared_ptr<session> *sp = nullptr;

                    if (!ec) {
                        sp = new std::shared_ptr<session>;
                        *sp = std::make_shared<session>(std::move(socket_), mysql, sessions);
                        sp->get()->init(this);
                        sessions->push_back(sp->get());
                        (*sp)->start();
                    } else {
                        cout << ec << std::endl;
                    }

                    do_accept();
                    delete sp;
                }
        );
    }
