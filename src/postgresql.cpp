#include "postgresql.h"
#include <iostream>
#include <boost/regex.hpp>
#include <map>

using namespace std;
using namespace nlohmann;

PostgreSQL::PostgreSQL(string connString) {
    c = new pqxx::connection(connString);
    initializeStatements();
}

PostgreSQL::PostgreSQL(PostgreSQL &psql){
    c = new pqxx::connection(psql.c->connection_string());
    initializeStatements();
}

PostgreSQL::~PostgreSQL(){
    c->close();
    delete c;
}

string PostgreSQL::toLowerCamelCase(std::string s)
{
    transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return tolower(c); });

    boost::regex re("([_-][a-z])");
    s = boost::regex_replace(s, re, [](const boost::smatch& sm){
        return string(1,toupper(sm[1].str().back()));
    });
    return s;
}

void PostgreSQL::initializeStatements(){
    //Formatted the string to copy/paste into the IDE with
    //https://tomeko.net/online_tools/cpp_text_escape.php?lang=en

    //All columns for content.vw_posts is specified here for clarity. The rest of the queries use *.
    string allPosts = "SELECT post_id\n"
                      "    ,game_id\n"
                      "    ,post_category_id\n"
                      "    ,post_uid\n"
                      "    ,post_title\n"
                      "    ,post_subtext\n"
                      "    ,post_preview_image_url\n"
                      "    ,post_datetime\n"
                      "    ,post_author\n"
                      "    ,post_markdown_content\n"
                      "    ,post_js_resource_key\n"
                      "    ,game_path\n"
                      "    ,post_category\n"
                      "FROM content.vw_posts vp\n";

    map<string,string> queries = {
        {"imageById","select image from content.fn_get_image_by_id($1)"},
        {"imageByFilename", "select image from content.fn_get_image_by_filename($1)"},
        {"postById","select * from content.fn_get_post_by_id($1)"},
        {"recentPosts","select * from content.fn_get_recent_posts($1)"},
        {"recentArticles","select * from content.fn_get_recent_articles($1)"},
        {"recentProjects","select * from content.fn_get_recent_projects($1)"},
        {"allProjects", allPosts + "WHERE vp.post_category = 'project'"},
        {"allArticles", allPosts + "WHERE vp.post_category = 'article'"}
    };

    for(auto &qry: queries){
        c->prepare(qry.first,qry.second);
    }
}

nlohmann::json PostgreSQL::pqxxFieldToJsonData(const pqxx::field &f){
    json data;
    if(f.is_null()){
        return data;
    }
    //will expand as needed.
    switch(f.type()){
    case 23: //int4
        data = static_cast<json>(f.as<int>());
        break;
    case 25://text
        data = static_cast<json>(f.as<string>());
        break;
    case 1043: //varchar
        data = static_cast<json>(f.as<string>());
        break;
    case 1114: //timestamp
        data = static_cast<json>(f.as<string>());
        break;
    default:
        data = static_cast<json>(f.as<string>());
        break;
    }
    return data;
}

nlohmann::json PostgreSQL::pqxxRowToJsonObject(const pqxx::row &r){
    //Define null interface.
    json obj = json::object();
    for(const auto &f:r){
        obj.emplace(toLowerCamelCase(f.name()),pqxxFieldToJsonData(f));
    }
    return obj;
}

nlohmann::json PostgreSQL::pqxxResultSetToJson(const pqxx::result &rs){
    json obj;
    if(rs.size() == 0){
        return  obj;
    } if(rs.size() == 1){
        return pqxxRowToJsonObject(rs.begin());
    } if(rs.size() > 1){
        obj = json::array();
        for(const auto &r:rs){
            obj.push_back(pqxxRowToJsonObject(r));
        }
        return obj;
    }
    return obj;
}

//QryParams are optional for the function.
template<typename... QryParams>
json PostgreSQL::execGenericJsonQry(std::string qry, QryParams... qprms){
    json data;
    try{
        pqxx::work txn{*c};
        pqxx::result r = txn.exec_prepared(qry,qprms...);
        data = pqxxResultSetToJson(r);
    }  catch(const exception &e){
        cout << e.what() << endl;
        throw;
    }
    return data;
}

template<typename... QryParams>
string PostgreSQL::execGenericImgQry(string qry, QryParams...qprms){
    string img;
    try{
        pqxx::work txn{*c};
        pqxx::result r = txn.exec_prepared(qry,qprms...);
        for(auto const &row: r){
            pqxx::binarystring bs = row[0].as<pqxx::binarystring>();
            img = bs.str();
        }
    } catch(const exception &e){
        cout << e.what() << endl;
        throw;
    }
    return img;
}

string PostgreSQL::getImageById(int id){
    return execGenericImgQry("imageById",id);
}

string PostgreSQL::getImageByFilename(string filename){
    return execGenericImgQry("imageByFilename",filename);
}

json PostgreSQL::getPostById(int id){
    return execGenericJsonQry("postById", id);
}

json PostgreSQL::getRecentPosts(int howMany){
    return execGenericJsonQry("recentPosts",howMany);
}

json PostgreSQL::getRecentArticles(int howMany){
    return execGenericJsonQry("recentArticles",howMany);
}

json PostgreSQL::getRecentProjects(int howMany){
    return execGenericJsonQry("recentProjects",howMany);
}

json PostgreSQL::getAllPosts(){
    return execGenericJsonQry("allPosts");
}

json PostgreSQL::getAllProjects(){
    return execGenericJsonQry("allProjects");
}

json PostgreSQL::getAllArticles(){
    return execGenericJsonQry("allArticles");
}
