
#include "server.h"
std::shared_ptr<session> *gsp = nullptr;

    server::server(short port, mysql_handler *_mysql, std::vector<session*> *_sessions, client *_client, gateway *_gateway)
            :
              socket_(io_service)
    {

        acceptor_ = new tcp::acceptor(io_service, tcp::endpoint(tcp::v4(), port));
        this->gateway_ = _gateway;
        sm_client = _client;
        sessions = _sessions;
        mysql = _mysql;
        do_accept();
        io_service.run();
    }

    char error_data[1000];
    std::size_t error_size;
    void server::do_accept()
    {
        acceptor_->async_accept(
                socket_,
                [this](system::error_code ec) {
                    std::shared_ptr<session> *sp = nullptr;

                    if (!ec) {
                        sp = new std::shared_ptr<session>;
                        *sp = std::make_shared<session>(std::move(socket_), mysql, sessions, sm_client, gateway_);
                       // gsp = sp;
                        sessions->push_back(sp->get());
                        (*sp)->start();
                    } else {
                        cerr << ec << std::endl;
                    }

                    do_accept();
                    delete sp;
                }
        );
    }
