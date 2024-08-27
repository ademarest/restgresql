#ifndef RESTAPI_H
#define RESTAPI_H

#include "mongoose/mongoose.h"
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <nlohmann/json.hpp>
#include "postgresql.h"

class RestAPI
{
public:
    RestAPI();
    RestAPI(bool ssl, std::string configFile);
    ~RestAPI();

    static void init();
    static void setSSL(bool ssl);
    static bool getSSL();
    static void setConfigFile(std::string configFile);
    static std::string getConfigFile();
    static nlohmann::json getConfigJson();

    static void createConfigFile(std::string configFile);
    static void handleRequest(struct mg_connection *c, int ev, void *ev_data);
    void startServer();

private:

    static inline std::unique_ptr<PostgreSQL> psql;
    static inline std::string configFile = "./restgresql.json";
    static inline std::mutex mtx;
    static inline bool ssl = true;
    bool running = true;
};

#endif // RESTAPI_H
