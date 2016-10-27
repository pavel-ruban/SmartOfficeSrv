#include "mysql_handler.h"

mysql_handler::mysql_handler() {
    hashes = new vector<string>;
    attributes = new vector<string>;
    types = new vector<string>;
    conn = new mysqlpp::Connection(true);
}

void mysql_handler::connect(string database, string address, string user, string password)
{
    conn->set_option(new mysqlpp :: SetCharsetNameOption ("utf8"));
    try {
        conn->connect(database.c_str(), address.c_str(), user.c_str(), password.c_str());
        connected = true;
    }
    catch (mysqlpp::Exception e) {
        cerr << e.what();
    }
}

mysql_handler::mysql_handler(string database, string address, string user, string password)
{
hashes = new vector<string>;
conn = new mysqlpp::Connection(true);
connect(database, address, user, password);
}

mysql_handler::~mysql_handler()
{
    delete conn;
    delete hashes;
    delete types;
}

void mysql_handler::refresh_hashes()
{
    if (connected) {
        try {
            mysqlpp::Query query = conn->query("SELECT hash from nodes");
            mysqlpp::StoreQueryResult res = query.store();
            if (res) {
                hashes->clear();
                mysqlpp::StoreQueryResult::const_iterator it;
                for (it = res.begin(); it != res.end(); ++it)
                {
                    mysqlpp::Row row = *it;
                    hashes->push_back(row[0].c_str());
                }
            }
        }
        catch (mysqlpp::Exception e) {
            cerr << e.what();
        }
    }
}

void mysql_handler::refresh()
{
    refresh_hashes();
    if (connected)
    {
        try
        {
            mysqlpp::Query query = conn->query("SELECT * from nodes");
            mysqlpp::StoreQueryResult res = query.store();
            if (res)
            {
                attributes->clear();
                mysqlpp::StoreQueryResult::const_iterator it;
                for (it = res.begin(); it != res.end(); ++it)
                {
                    mysqlpp::Row row = *it;
                    //cout << row[1] << " " << row[2] <<  " " << row[3] <<  " " << row[4] << std::endl;
                    boost::property_tree::ptree pt;
                    std::stringstream ss;
                    ss.str(row[5].c_str());
                    if (strlen(row[5].c_str()) != 0) {
                        boost::property_tree::read_json(ss, pt);
                        if (pt.get<bool>("default")) {
                            default_ip = row[2].c_str();
                            default_port = atoi(row[3].c_str());
                        }
                    }
                    types->push_back(row[4].c_str());
                    attributes->push_back(row[5].c_str());
                }
            }
        }
        catch (mysqlpp::Exception e) {
            cerr << e.what();
        }
    }
}

std::pair<string, short> mysql_handler::get_default_host()
{
    return std::make_pair(default_ip, default_port);
}

string mysql_handler::get_attributes(string hash)
{
    int i = 0;
    for (auto it = hashes->begin(); it != hashes->end(); ++it, i++)
    {
        if ((*it) == hash)
        {
            return (*attributes)[i];
        }
    }
}

string mysql_handler::get_type(string hash)
{
    int i = 0;
    for (auto it = hashes->begin(); it != hashes->end(); ++it, i++)
    {
        if ((*it) == hash)
        {
            return (*types)[i];
        }
    }
}

void mysql_handler::print_hashes()
{
    for (auto it = hashes->begin(); it != hashes->end(); ++it)
    {
        cout << *it << endl;
    }
}

std::pair<string, short> mysql_handler::get_host_by_id(string node_id)
{
    if (connected) {
        try {
            mysqlpp::Query query = conn->query("SELECT * FROM `nodes` WHERE hash='" + node_id + "'");
            mysqlpp::StoreQueryResult res = query.store();
            if (res) {
                if(res.begin() != res.end()) {
                    mysqlpp::Row row = *res.begin();
                    return std::make_pair(row[2].c_str(), atoi(row[3].c_str()));
                } else return std::make_pair("",666); // Takie dela.
            }
        }
        catch (mysqlpp::Exception e) {
            cerr << e.what();
        }
    }
}


//    void mysql_handler::print_tree(boost::property_tree::ptree const& pt)
//    {
//        using boost::property_tree::ptree;
//        ptree::const_iterator end = pt.end();
//        for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
//            std::cout << it->first << ": " << it->second.get_value<std::string>() << std::endl;
//            print_tree(it->second);
//        }
//    }

bool mysql_handler::is_user_exists(string hash)
{
    return std::find(hashes->begin(), hashes->end(), hash) != hashes->end();
}

