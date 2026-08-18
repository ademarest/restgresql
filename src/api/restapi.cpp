#include "restapi.h"
#include "db/postgresql.h"
#include "router.h"
#include <iostream>
#include <fstream>
#include <mutex>

using namespace nlohmann;
using namespace std;
using namespace boost;

RestAPI::RestAPI(){
    init();
}

RestAPI::RestAPI(bool ssl, string configFile) {
    if(!ssl){
        cout << "Warning: Running without SSL" << endl;
    }
    RestAPI::ssl = ssl;
    RestAPI::setConfigFile(configFile);
    init();
}


RestAPI::~RestAPI() {
    running=false;
}

void RestAPI::init(){
    if(!boost::filesystem::exists(RestAPI::configFile)){
        createConfigFile(configFile);
    }
    //Cache the config so handleRequest does not re-read the file on every event.
    RestAPI::config = RestAPI::getConfigJson();
    psql.reset(new PostgreSQL(config["dbConnString"]));
}

void RestAPI::setSSL(bool ssl){
    lock_guard<mutex> guard(RestAPI::mtx);
    RestAPI::ssl = ssl;
}

bool RestAPI::getSSL(){
    return RestAPI::ssl;
}

void RestAPI::setConfigFile(std::string configFile)
{
    //Do not want multiple threads changing the static class member.
    lock_guard<mutex> guard(RestAPI::mtx);
    RestAPI::configFile = configFile;
}

string RestAPI::getConfigFile()
{
    return RestAPI::configFile;
}

json RestAPI::getConfigJson(){
    string configFile = RestAPI::configFile;
    std::ifstream cf(configFile);
    json data = json::parse(cf);
    cf.close();
    return data;
}

void RestAPI::createConfigFile(string configFile){

    try{
        boost::filesystem::path path{configFile};
        boost::filesystem::create_directories(path.parent_path());
    }
    catch(const std::exception &e){
        cout << e.what() << endl;
        exit(-1);
    }

    string dbConnString;
    string certPath;
    string keyPath;
    cout << "Please enter your PostgresSQL database connection string: ";
    getline(cin,dbConnString);
    cout << endl;

    cout << "Please enter the filesystem path to your server's SSL certificate file: ";
    getline(cin,certPath);
    cout << endl;

    cout << "Please enter the filesystem path to your server's SSL key file: ";
    getline(cin,keyPath);
    cout << endl;

    json config = json::object(
        {
            {"dbConnString",dbConnString},
            {"certPath",certPath},
            {"keyPath",keyPath}
        }
        );

    ofstream cf(configFile);
    cf << config.dump(4).c_str();
    cf.close();
}

void RestAPI::sendJson(struct mg_connection *c, std::string_view body) {
    mg_printf(c,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type:application/json\r\n"
              "Access-Control-Allow-Origin:*\r\n"
              "Content-Length:%lu\r\n\r\n",
              (unsigned long) body.size());
    mg_send(c, body.data(), body.size());
    c->is_resp = 0;
}

void RestAPI::sendImage(struct mg_connection *c, const std::string &img) {
    if(img.empty()){
        mg_http_reply(c, 404, "", "Image not found\n");
        return;
    }
    mg_printf(c,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type:application/octet-stream\r\n"
              "Access-Control-Allow-Origin:*\r\n"
              "Content-Length:%lu\r\n\r\n",
              (unsigned long) img.size());
    mg_send(c, img.data(), img.size());
    c->recv.len = 0;     // Clean receive buffer
    c->is_draining = 1;  // Close this connection when the response is sent
}

void RestAPI::handleRequest(struct mg_connection *c, int ev, void *ev_data){
    if (ev == MG_EV_ACCEPT && RestAPI::ssl) {
        string certPath = config.value("certPath","");
        string keyPath = config.value("keyPath","");
        mg_str crt = mg_file_read(&mg_fs_posix, certPath.c_str());
        mg_str key = mg_file_read(&mg_fs_posix, keyPath.c_str());
        struct mg_tls_opts opts = {.cert = crt, .key = key};
        mg_tls_init(c, &opts);
    }

    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *message = (struct mg_http_message *) ev_data;

        string uri(message->uri.buf,message->uri.len);
        Router::Route route = Router::match(uri);

        try{
            switch(route.endpoint){
            case Router::Endpoint::AllPosts:
                sendJson(c, psql->getAllPosts().dump(4));
                break;
            case Router::Endpoint::AllProjects:
                sendJson(c, psql->getAllProjects().dump(4));
                break;
            case Router::Endpoint::AllArticles:
                sendJson(c, psql->getAllArticles().dump(4));
                break;
            case Router::Endpoint::PostById:
                sendJson(c, psql->getPostById(route.intParam).dump(4));
                break;
            case Router::Endpoint::RecentPosts:
                sendJson(c, psql->getRecentPosts(route.intParam).dump(4));
                break;
            case Router::Endpoint::RecentArticles:
                sendJson(c, psql->getRecentArticles(route.intParam).dump(4));
                break;
            case Router::Endpoint::RecentProjects:
                sendJson(c, psql->getRecentProjects(route.intParam).dump(4));
                break;
            case Router::Endpoint::ImageById:
                sendImage(c, psql->getImageById(route.intParam));
                break;
            case Router::Endpoint::ImageByFilename:
                sendImage(c, psql->getImageByFilename(route.strParam));
                break;
            case Router::Endpoint::NotFound:
            default:
                mg_http_reply(c, 404, "", "<h1>404</h1>\nYour call cannot be completed as dialed.\nPlease hang up and try again.\n");
                break;
            }

        } catch(const pqxx::broken_connection &e){
            //Only a broken connection warrants re-establishing the DB session.
            cout << "RestAPI::handleRequest " << e.what() << endl;
            try{
                psql.reset(new PostgreSQL(config["dbConnString"]));
            } catch(const std::exception &re){
                cout << "RestAPI::handleRequest reconnect failed: " << re.what() << endl;
            }
            mg_http_reply(c, 500, "", "<h1>500</h1>\nInternal Server Error!\n I don't feel so good Mr. Stark.");
        } catch(const std::exception &e){
            cout << "RestAPI::handleRequest " << e.what() << endl;
            mg_http_reply(c, 500, "", "<h1>500</h1>\nInternal Server Error!\n I don't feel so good Mr. Stark.");
        }
    }
}

void RestAPI::startServer(){
    mg_log_set(MG_LL_DEBUG);
    struct mg_mgr mgr;  // Declare event manager
    mg_mgr_init(&mgr);  // Initialise event manager
    mg_http_listen(&mgr, "http://0.0.0.0:8000", handleRequest, NULL);  // Setup listener
    while (running == true) mg_mgr_poll(&mgr, 1000);   // Event loop
    mg_mgr_free(&mgr);
}
