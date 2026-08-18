#ifndef ROUTER_H
#define ROUTER_H

#include <string>

namespace Router {

    enum class Endpoint {
        AllPosts,
        AllProjects,
        AllArticles,
        PostById,
        RecentPosts,
        RecentArticles,
        RecentProjects,
        ImageById,
        ImageByFilename,
        NotFound
    };

    struct Route {
        Endpoint endpoint = Endpoint::NotFound;
        int intParam = 0;         //Set for PostById, Recent*, and ImageById.
        std::string strParam;     //Set for ImageByFilename.
    };

    Route match(const std::string &uri);
}

#endif // ROUTER_H
