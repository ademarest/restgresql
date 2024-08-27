#include "restapi.h"
#include "postgresql.h"
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
    json data = RestAPI::getConfigJson();
    psql.reset(new PostgreSQL(data["dbConnString"]));
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

void RestAPI::handleRequest(struct mg_connection *c, int ev, void *ev_data){
    json data = RestAPI::getConfigJson();

    string dbConnString = data["dbConnString"];
    string certPath = data["certPath"];
    string keyPath = data["keyPath"];

    if (ev == MG_EV_ACCEPT && RestAPI::ssl) {
        mg_str crt = mg_file_read(&mg_fs_posix, certPath.c_str());
        mg_str key = mg_file_read(&mg_fs_posix, keyPath.c_str());
        struct mg_tls_opts opts = {.cert = crt, .key = key};
        mg_tls_init(c, &opts);
    }

    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *message = (struct mg_http_message *) ev_data;

        string uri(message->uri.buf,message->uri.len);

        regex postByIdExp("(/api/posts/(\\d+$))");
        regex recentPostsExp("(/api/recentPosts/(\\d+$))");
        regex recentArticlesExp("(/api/recentArticles/(\\d+$))");
        regex recentProjectsExp("(/api/recentProjects/(\\d+$))");
        regex imageByIdExp("(/api/images/(\\d+$))");
        regex imageByFilenameExp("(/api/images/([^\\s]+(\\.(?i)(jpeg|jpg|png|gif|bmp))$))");

        smatch postByIdMatch;
        smatch recentPostsMatch;
        smatch recentArticlesMatch;
        smatch recentProjectsMatch;
        smatch imageByIdMatch;
        smatch imageByFilenameMatch;

        try{
            //In Mongoose 7.13 this was:
            /*mg_http_match_uri(message, "/api/articles")*/

            //All posts
            if(mg_match(message->uri,mg_str("/api/posts"), NULL)){

                mg_http_reply(c,
                              200,
                              "Content-Type:application/json\r\n"
                              "Access-Control-Allow-Origin:*\r\n",
                              "%s\n", psql->getAllPosts().dump(4).c_str()
                              );

            }
            //All projects
            else if(mg_match(message->uri,mg_str("/api/projects"),NULL)){

                mg_http_reply(c,
                              200,
                              "Content-Type:application/json\r\n"
                              "Access-Control-Allow-Origin:*\r\n",
                              "%s\n", psql->getAllProjects().dump(4).c_str()
                              );

            }
            //All articles
            else if(mg_match(message->uri,mg_str("/api/articles"),NULL)){

                mg_http_reply(c,
                              200,
                              "Content-Type:application/json\r\n"
                              "Access-Control-Allow-Origin:*\r\n",
                              "%s\n", psql->getAllArticles().dump(4).c_str()
                              );

            }
            //Post by id
            else if(regex_search(uri,postByIdMatch,postByIdExp)){

                int postId = stoi(postByIdMatch[2]);

                mg_http_reply(c,
                              200,
                              "Content-Type:application/json\r\n"
                              "Access-Control-Allow-Origin:*\r\n",
                              "%s\n", psql->getPostById(postId).dump(4).c_str()
                              );

            }
            //Recent Posts
            else if(regex_search(uri,recentPostsMatch,recentPostsExp)){
                int howMany = stoi(recentPostsMatch[2]);

                mg_http_reply(c,
                              200,
                              "Content-Type:application/json\r\n"
                              "Access-Control-Allow-Origin:*\r\n",
                              "%s\n", psql->getRecentPosts(howMany).dump(4).c_str()
                              );

            }
            //Recent articles
            else if(regex_search(uri,recentArticlesMatch,recentArticlesExp)){
                int howMany = stoi(recentArticlesMatch[2]);

                mg_http_reply(c,
                              200,
                              "Content-Type:application/json\r\n"
                              "Access-Control-Allow-Origin:*\r\n",
                              "%s\n", psql->getRecentArticles(howMany).dump(4).c_str()
                              );

            }
            //Recent projects
            else if(regex_search(uri, recentProjectsMatch,recentProjectsExp)){
                int howMany = stoi(recentProjectsMatch[2]);

                mg_http_reply(c,
                              200,
                              "Content-Type:application/json\r\n"
                              "Access-Control-Allow-Origin:*\r\n",
                              "%s\n", psql->getRecentProjects(howMany).dump(4).c_str()
                              );

            }
            //Image by id
            else if(regex_search(uri,imageByIdMatch,imageByIdExp)){
                int imageId = stoi(imageByIdMatch[2]);
                string img = psql->getImageById(imageId);
                int buffer = img.size();

                mg_printf(c, "HTTP/1.1 200 OK\r\nContent-Length:%d\r\n\r\n", buffer);
                mg_send(c, img.data(), buffer);
                c->recv.len = 0;     // Clean receive buffer
                c->is_draining = 1;  // Close this connection when the response is sent

                //Image by file name
            } else if(regex_search(uri,imageByFilenameMatch, imageByFilenameExp)){

                string imageFileName = imageByFilenameMatch[2];
                string img = psql->getImageByFilename(imageFileName);
                int buffer = img.size();

                mg_printf(c, "HTTP/1.1 200 OK\r\nContent-Length:%d\r\n\r\n", buffer);
                mg_send(c, img.data(), buffer);
                c->recv.len = 0;     // Clean receive buffer
                c->is_draining = 1;  // Close this connection when the response is sent

            }
            //404
            else {
                mg_http_reply(c, 404, "", "<h1>404</h1>\nYour call cannot be completed as dialed.\nPlease hang up and try again.\n");
            }

        } catch(const std::exception &e){
            cout << e.what() << endl;
            psql.reset(new PostgreSQL(dbConnString));
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
