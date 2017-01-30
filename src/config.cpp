#include "config.h"



config::config(std::string filename, mysql_handler &mysql) {
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(filename, pt);
    istringstream (pt.get<string>("General.port")) >> port;
    istringstream (pt.get<string>("MySQL.database")) >> database;
    istringstream (pt.get<string>("MySQL.address")) >> server_address;
    istringstream (pt.get<string>("MySQL.user")) >> user;
    istringstream (pt.get<string>("MySQL.password")) >> password;
    mysql.connect(database, server_address, user, password);
    mysql.refresh();
    vars = mysql.get_config();
    initialized = true;
}

config::~config(){

}

std::map <std::string, std::string> config::get_vars() {
    return vars;
}

short config::get_port() {
    return this->port;
}

std::string config::get_variable(std::string name) {
    if (!initialized) {
        throw not_initialized();
    }
    if ( vars.find(name) == vars.end() ) {
        throw empty_var(name);
    } else {
        return vars[name];
    }
}
