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
            if (_mysql->is_user_exists(buf[1])) {
                if (_mysql->get_type(buf[1]) == "bo") {
                    native_to_http bo(req, _config);
                    return bo.convert();
                }
            }
            return req;
        }
    }
    return "42";
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
        buf.clear();
        split(buf,*it,is_any_of(":"));
        if (buf[0] != "" && buf.size() > 1)
            headers[buf[0]] = buf[1].substr(1,buf[1].size());
    }
    return headers;
}
native_to_http::~native_to_http() {

}

native_to_http::native_to_http(std::string request, config *_config){
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
        } else {
            result = get_html_headers(headers["action"], headers["destination"]);
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

