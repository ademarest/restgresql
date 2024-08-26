#include "restapi.h"
#include <iostream>
#include <boost/program_options.hpp>

using namespace boost::program_options;
// HTTP server event handler function

int main(int argc, char *argv[]) {
    //For Archlinux, the following additional packaged are required.
    //https://archlinux.org/packages/extra/x86_64/boost/
    //https://archlinux.org/packages/extra/x86_64/git/
    //https://archlinux.org/packages/extra/x86_64/libpqxx/
    //https://archlinux.org/packages/core/x86_64/openssl/
    //https://archlinux.org/packages/extra/x86_64/cmake/

    //For Ubuntu Server 24.04 LTS, the following are needed.
    //cmake
    //libboost-all-dev
    //libssl-dev
    //libpqxx-dev
    //libbz2-dev
    //liblzma-dev
    //libzstd-dev

    std::string configFile;
    options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Usage: restgresql [OPTION...]")
        (
            "config,c",
            value<std::string>()->default_value("/etc/restgresql/restgresql.conf")->value_name("[=CONFIG.json]"),
            "Set the configuration file for the RestgreSQL."
        );

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if(vm.count("help")){
        std::cout << desc << "\n";
        return 0;
    }

    RestAPI *api = new RestAPI(vm["config"].as<std::string>());
    api->startServer();
    return 0;
}
