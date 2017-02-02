#pragma once
#include "server.h"
#include "session.h"

class response_handler
{
private:
    server *_server;
    std::vector<session*> *_sessions;
public:
    response_handler(server *server, std::vector<session*> *sessions);
    ~response_handler();
    string handle(string responce, size_t length, session *session);
};
