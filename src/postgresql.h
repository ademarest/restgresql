#ifndef POSTGRESQL_H
#define POSTGRESQL_H

#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <stdarg.h>

class PostgreSQL
{
public:
    //Rule of 3
    PostgreSQL(std::string connString);
    PostgreSQL(PostgreSQL &psql);
    ~PostgreSQL();

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

    template<typename... QryParams>
    nlohmann::json execGenericJsonQry(std::string qry, QryParams... qprms);

    template<typename... QryParams>
    std::string execGenericImgQry(std::string qry, QryParams... qprms);

private:
    pqxx::connection *c = nullptr;

    void initializeStatements();
    nlohmann::json pqxxFieldToJsonData(const pqxx::field &f);
    nlohmann::json pqxxRowToJsonObject(const pqxx::row &r);
    nlohmann::json pqxxResultSetToJson(const pqxx::result &rs);
};

#endif // POSTGRESQL_H
