#ifndef POSTGRESQL_H
#define POSTGRESQL_H

#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <stdarg.h>
#include <memory>
#include <mutex>

class PostgreSQL
{
public:
    PostgreSQL(std::string connString);

    std::string toLowerCamelCase(std::string in);

    std::string getImageById(int id);
    std::string getImageByFilename(std::string filename);
    nlohmann::json getPostById(int id);
    nlohmann::json getRecentPosts(int howMany);
    nlohmann::json getRecentArticles(int howMany);
    nlohmann::json getRecentProjects(int howMany);
    nlohmann::json getAllArticles();
    nlohmann::json getAllPosts();
    nlohmann::json getAllProjects();

    nlohmann::json execGenericJsonArrQry(std::string qry,  pqxx::params qprms = {});
    nlohmann::json execGenericJsonObjQry(std::string qry,  pqxx::params qprms = {});
    std::string    execGenericImgQry(std::string qry,      pqxx::params qprms);

private:
    std::shared_ptr<std::mutex> cmtx{std::make_shared<std::mutex>()};
    std::shared_ptr<pqxx::connection> c;

    void initializeStatements();
    nlohmann::json pqxxFieldToJsonData(const pqxx::field &f);
    nlohmann::json pqxxRowToJsonObject(const pqxx::row &r);
    nlohmann::json pqxxResultSetToJson(const pqxx::result &rs, bool isArr);
    nlohmann::json execGenericJsonQry(std::string qry, bool isArr, pqxx::params qprms = {});
};

#endif // POSTGRESQL_H
