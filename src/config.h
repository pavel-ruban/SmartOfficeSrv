#pragma once
#include <string>
#include <map>
#include <mysql_handler.h>
#include <boost/property_tree/ini_parser.hpp>

class empty_var : public std::exception
{
    std::string s;
public:
    empty_var(std::string ss) : s(ss) {}
    ~empty_var() throw () {}
    const char* what() const throw() {
        return ("Setting " + s + " not found.").c_str();
    }
};

class not_initialized : public std::exception
{
    //bad_request();
    //~bad_request() throw () {}
    virtual const char* what() const throw()
    {
        return "Config not initialized.";
    }
};

class config {
private:
    bool initialized = false;
    std::map <std::string, std::string> vars;
    short port;
    std::string database, server_address, user, password;
    string _bearer = "";
    //config(config const&); // реализация не нужна
    //config& operator= (config const&);  // и тут
public:
    config();
    std::map <std::string, std::string> get_vars();
    ~config();
    config(std::string filename, mysql_handler &mysql);
    short get_port();
    string get_bearer();
    void set_bearer(string bearer);
    std::string get_variable(std::string name);
};


