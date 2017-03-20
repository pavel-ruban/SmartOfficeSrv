#include "logger.h"


logger::logger(std::ostream &log, std::ostream &error)
{
    _log = &log;
    _error = &error;
}

void logger::log(std::string message) {
    *_log << "[LOG]: " << message << std::endl;
}

void logger::log(std::string id, std::string message) {
    *_log << "[LOG](" + id + "): " << message << std::endl;
}

void logger::clients_amount(unsigned int amount)
{
    *_log << "[LOG]: Current amount of clients: " << amount << "." << std::endl;
}

void logger::log_response(std::string ip, std::string session_id, std::string response)
{
    if (session_id != "")
    {
        *_log << "[LOG]: Recieved from: " + session_id + " (Ip: " + ip + " )" << std::endl;
        *_log << "----------------------------------------" << "\n" << response << std::endl;
        *_log << "----------------------------------------" << std::endl;
    } else
    {
        *_log << "[LOG]: Recieved from: ip: " + ip + "" << std::endl;
        *_log << "----------------------------------------" << "\n" << response << std::endl;
        *_log << "----------------------------------------" << std::endl;
    }
}

void logger::log_request(std::string dest_ip, std::string session_id, std::string request, unsigned int timeout)
{
    if (session_id != "")
    {
        *_log << "[LOG]: Trying to send to: " + session_id;
        if (dest_ip != "")
            *_log << " (Ip: " + dest_ip + " )";
        if (timeout)
            *_log << " Timeout: " << timeout;
        *_log << std::endl;
        *_log << "----------------------------------------" << "\n" << request << std::endl;
        *_log << "----------------------------------------" << std::endl;
    } else
    {
        *_log << "[LOG]: Trying to send to: " + dest_ip + "";
        if (timeout)
            *_log << " Timeout: " << timeout;
        *_log << std::endl;
        *_log << "----------------------------------------" << "\n" << request << std::endl;
        *_log << "----------------------------------------" << std::endl;
    }
}