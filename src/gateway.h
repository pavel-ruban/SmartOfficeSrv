#pragma once

#include <string>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <exception>
#include <config.h>

using namespace boost;
using namespace std;

class bad_request : public std::exception
{
    //bad_request();
    //~bad_request() throw () {}
    virtual const char* what() const throw()
    {
        return "Bad request. No action/destination header.";
    }
};

class gateway
{
public:
    std::string convert(string request);
    std::string magic(string req);
    config* get_config();
    //void bind_mysql();
    gateway(mysql_handler *mysql, config *_config);
    ~gateway() {  }
private:
    mysql_handler *_mysql;
    config *_config;
};


class converter
{
protected:
    virtual map<string, string> parse_headers(string data_to_parse) = 0;
public:
    virtual ~converter() {}
    virtual std::string convert() = 0;
};

class native_to_http : public converter
{
protected:
    map<string, string> parse_headers(string data_to_parse);
private:
    std::string request;
    config *_config;
    map<string, string> conf_vars;
    string get_html_headers(string action, string destination);
public:
    virtual ~native_to_http();
    native_to_http(std::string request, config *_config);
    virtual std::string convert();

};

class http_to_native : public converter
{
protected:
    map<string, string> parse_headers(string data_to_parse);
private:
    std::string request;
    config *_config;
    map<string, string> conf_vars;
public:
    virtual ~http_to_native();
    http_to_native(std::string request, config *_config);
    virtual std::string convert();
};