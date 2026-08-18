#include "router.h"
#include <boost/regex.hpp>

using namespace std;

namespace {
    //Returns false for non-numeric or out-of-int-range input instead of throwing.
    bool parseInt(const string &s, int &out){
        try {
            out = stoi(s);
            return true;
        } catch (const exception&) {
            return false;
        }
    }
}

namespace Router {

    Route match(const string &uri){
        //Anchored with regex_match so partial paths like /evil/api/posts/1 do not match.
        static const boost::regex postByIdExp("/api/posts/(\\d+)");
        static const boost::regex recentPostsExp("/api/recentPosts/(\\d+)");
        static const boost::regex recentArticlesExp("/api/recentArticles/(\\d+)");
        static const boost::regex recentProjectsExp("/api/recentProjects/(\\d+)");
        static const boost::regex imageByIdExp("/api/images/(\\d+)");
        static const boost::regex imageByFilenameExp("/api/images/([^\\s/]+\\.(?i:jpeg|jpg|png|gif|bmp))");

        Route route;
        boost::smatch m;

        if(uri == "/api/posts"){
            route.endpoint = Endpoint::AllPosts;
        }
        else if(uri == "/api/projects"){
            route.endpoint = Endpoint::AllProjects;
        }
        else if(uri == "/api/articles"){
            route.endpoint = Endpoint::AllArticles;
        }
        else if(boost::regex_match(uri, m, postByIdExp)){
            if(parseInt(m[1], route.intParam)) route.endpoint = Endpoint::PostById;
        }
        else if(boost::regex_match(uri, m, recentPostsExp)){
            if(parseInt(m[1], route.intParam)) route.endpoint = Endpoint::RecentPosts;
        }
        else if(boost::regex_match(uri, m, recentArticlesExp)){
            if(parseInt(m[1], route.intParam)) route.endpoint = Endpoint::RecentArticles;
        }
        else if(boost::regex_match(uri, m, recentProjectsExp)){
            if(parseInt(m[1], route.intParam)) route.endpoint = Endpoint::RecentProjects;
        }
        else if(boost::regex_match(uri, m, imageByIdExp)){
            if(parseInt(m[1], route.intParam)) route.endpoint = Endpoint::ImageById;
        }
        else if(boost::regex_match(uri, m, imageByFilenameExp)){
            route.strParam = m[1];
            route.endpoint = Endpoint::ImageByFilename;
        }

        return route;
    }
}
