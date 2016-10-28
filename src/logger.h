#pragma once

#include <iosfwd>
#include <ostream>

class logger {
public:
    logger(std::ostream &log, std::ostream &error);
    void log(std::string message);
    void log(std::string id, std::string message);
    void clients_amount(unsigned int amount);
    void log_response(std::string ip, std::string session_id, std::string response);
    void log_request(std::string dest_ip, std::string session_id, std::string request, unsigned int timeout);
private:
    std::ostream *_log;
    std::ostream *_error;
};

