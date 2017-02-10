#include <stdio.h>      /* printf, fgets */
#include <stdlib.h>
#include "gateway.h"


gateway::gateway(mysql_handler *mysql, config *_config) {
    _mysql = mysql;
    this->_config = _config;
}

std::string gateway::magic(string req) {
    vector<string> strs;
    split(strs, req, is_any_of("\n"));

    typedef std::vector<string>::iterator it_type;
    for(it_type iterator = strs.begin(); iterator != strs.end(); iterator++) {
        if (iterator->find("destination") != std::string::npos) {
            vector<string> buf;

            string b_s = iterator->c_str();
            split(buf, b_s, is_any_of(" "));
            if (buf[1][buf[1].length() - 1] == '\r')
                buf[1] = buf[1].substr(0, buf[1].length() - 1);
            if (_mysql->is_user_exists(buf[1])) {
                if (_mysql->get_type(buf[1]) == "bo") {
                    native_to_http bo(req, _config);
                    return bo.convert();
                }
                if (_mysql->get_type(buf[1]) == "pcd") {
                    http_to_native pcb(req, _config);
                    string buf_test = pcb.convert();
                    return buf_test;
                }
            }
            return req;
        }
    }
    return "42";
}

config* gateway::get_config() { //Костыль.
    return _config;
}

std::string gateway::convert(string request) {
//    vector<string> strs;
//    split(strs, request, is_any_of("\n"));

//    typedef std::vector<string>::iterator it_type;
//    for(it_type iterator = strs.begin(); iterator != strs.end(); iterator++) {
//        if (iterator->find("destination") != std::string::npos) {
//            vector<string> buf;
//            string b_s = iterator->c_str();
//            split(buf, b_s, is_any_of(" "));
//            string test = _mysql->get_type(buf[1]);
//            int i = 0;
//        }
//    }
}



//-----------------------------------------------------------------------------
map<string, string> native_to_http::parse_headers(string data_to_parse) {
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
        size_t delim_pos = (*it).find(": ");
        if (delim_pos == string::npos)
            continue;
        string header_name = (*it).substr(0, delim_pos);
        string header_val = (*it).substr(delim_pos + 2, (*it).length() - delim_pos);
        if (header_name != "" && header_val != "")
            headers[header_name] = header_val;
    }
    return headers;
}
native_to_http::~native_to_http() {

}

native_to_http::native_to_http(std::string request, config *_config) {
    this->_config = _config;
    this->request = request;
}

string native_to_http::get_html_headers(string action, string destination) {
    string buf = _config->get_variable("API:" + destination + ":" + action);
    vector<string> strs;
    split(strs, buf, is_any_of(":"));
    string host = _config->get_variable("API:" + destination + ":host");
    return strs[0] + " " + strs[1] + " HTTP/1.0\nHost:" + host + "\n";
}

std::string native_to_http::convert() {
    string result = "";
    auto headers = this->parse_headers(request);
    if ( headers.find("action") == headers.end() ) {
        throw bad_request();
    } else {
        if ( headers.find("destination") == headers.end() ) {
            throw bad_request();
        }
        else
        {
            if (headers.find("uid") != headers.end() ) {
                string buf = headers["uid"];
                vector<string> strs;
                split(strs, buf, is_any_of("-"));
                char hex_buf[20] = {0};

                sprintf(
                        hex_buf,
                        "%X:%X:%X:%X",
                        atoi(strs[0].c_str()),
                        atoi(strs[1].c_str()),
                        atoi(strs[2].c_str()),
                        atoi(strs[3].c_str())
                );

                headers["uid"] = string(hex_buf);
            }

            result = get_html_headers(headers["action"], headers["destination"]);

            if (headers["destination"] == "bo_test") {
                result += "Authorization:Api-key " + _config->get_variable("API:auth:Api-key") + "\n";
//                if (_config->get_bearer() != "") {
//                    result += "X-Api-Authorization:Bearer " + _config->get_bearer() + "\n";
//                }
            }

            typedef std::map<std::string, std::string>::iterator it_type;
            for(it_type iterator = headers.begin(); iterator != headers.end(); iterator++) {
                // iterator->first = key
                // iterator->second = value
                result += iterator->first + ": " + iterator->second + "\n";
            }
            result+="\n";

        }
    }
    //
    return result;
}

//-----------------------------------------------------------------------------
http_to_native::http_to_native(std::string request, config *_config) {
    this->_config = _config;
    this->request = request;
}

http_to_native::~http_to_native() {

}

map<string, string> http_to_native::parse_headers(string data_to_parse) {
    vector<string> strs;
    split(strs, data_to_parse, is_any_of("\n"));
    map<string,string> headers;

    for(vector<string>::iterator it = strs.begin(); it != strs.end(); ++it)
    {
        if ((*it).find("HTTP/1.1") != string::npos) {
            vector<string> st;
            string str = (*it);
            split(st, str, boost::is_any_of(" "));
            headers["status"] = st[1];
            continue;
        }
        size_t delim_pos = (*it).find(": ");
        if (delim_pos == string::npos)
            continue;
        string header_name = (*it).substr(0, delim_pos);
        string header_val = (*it).substr(delim_pos + 2, (*it).length() - delim_pos - 3);
        if (header_name[0] > 0x40 && header_name[0] < 0x5B)
            continue;
        if (header_name == "uid") {
            vector<string> st;
            string str = header_val;
            split(st, str, boost::is_any_of(":"));
            char buf_c[20] = {0};
            uint8_t uid[4];
            for (uint8_t i = 0; i < 4; ++i) {
                uid[i] = strtoul(st[i].substr(0, 2).c_str(), NULL, 16);
            }
            sprintf(buf_c, "%d-%d-%d-%d", uid[0], uid[1], uid[2], uid[3]);
            header_val = string(buf_c);
        }
        if (header_name != "" && header_val != "")
            headers[header_name] = header_val;
    }
    if (data_to_parse.find("\"invalidate\":1") != string::npos) {
        string inv_to = "";
        if (data_to_parse.find("\"access\":0") != string::npos)
            inv_to = "denied";
        if (data_to_parse.find("\"access\":1") != string::npos)
            inv_to = "granted";
        headers["invalidate-cache"] = inv_to;
    }
    return headers;
}

std::string http_to_native::convert() {
    string result = "";
    auto headers = this->parse_headers(request);
    typedef std::map<std::string, std::string>::iterator it_type;
    for(it_type iterator = headers.begin(); iterator != headers.end(); iterator++) {
        result += iterator->first + ": " + iterator->second + "\n";
    }
    result+="\n\n";
    return result;
}