#include "response_handler.h"


response_handler::response_handler(server *server, std::vector<session*> *sessions) {
    _server = server;
    _sessions = sessions;
}

response_handler::~response_handler() {
    delete _server;
    delete _sessions;
}

string response_handler::handle(string data, size_t length, session *session) {

    vector<string> strs;
    vector<string> buf;

    string headers_s;

    map<string,string> headers;

    typedef map<string, int, less<string> > map_type;
    regex expression("^(.+(?:\r\n|\n|\r))+(?:\r\n|\n|\r)(.*)$");

    string::const_iterator start, end;


    start = data.begin();
    end = data.begin() + length;

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
        headers[buf[0]] = buf[1].substr(1,buf[1].size());
    }

    // Если запрос не содержит идентификатор.
    if (headers.find("node_id") == headers.end()) {
        strcpy(session->transmitted_data_, "status: 400\n");
    } else {
        if (headers.find("node_id") != headers.end()) {
            *session->node_id = (*headers.find("node_id")).second;
        } else
            *session->node_id = "";

        session->mysql->refresh();
        if (session->mysql->is_user_exists(*session->node_id)) {
            if (headers["action"] == "call")
                if(headers.find(headers["destination"]) != headers.end()) {
//                        send_message()
                }
            // Звонок.
            cout << "RING" << endl;
            strcpy(session->transmitted_data_, "status: 200\n");
        } else
            strcpy(session->transmitted_data_, "status: 401\n");

    }
}