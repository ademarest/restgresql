#include "utils/utils.h"
#include "api/router.h"
#include <iostream>
#include <string>
#include <vector>
#include <cassert>

struct TestCase {
    std::string input;
    std::string expected;
};

void test_toLowerCamelCase() {
    std::cout << "Running test_toLowerCamelCase..." << std::endl;

    std::vector<TestCase> cases = {
        {"post_id", "postId"},
        {"POST_ID", "postId"},
        {"post_category_id", "postCategoryId"},
        {"post_preview_image_url", "postPreviewImageUrl"},
        {"alreadyCamelCase", "alreadyCamelCase"},
        {"simple", "simple"},
        {"with-hyphen", "withHyphen"},
        {"multiple__underscores", "multipleUnderscores"},
        {"", ""}
    };

    for (const auto& tc : cases) {
        std::string result = Utils::toLowerCamelCase(tc.input);
        if (result != tc.expected) {
            std::cerr << "Test Failed: Input '" << tc.input 
                      << "' expected '" << tc.expected 
                      << "' but got '" << result << "'" << std::endl;
            std::exit(1);
        }
    }
    std::cout << "test_toLowerCamelCase passed!" << std::endl;
}

struct RouteCase {
    std::string uri;
    Router::Endpoint expected;
    int intParam = 0;
    std::string strParam;
};

void test_router() {
    std::cout << "Running test_router..." << std::endl;

    std::vector<RouteCase> cases = {
        //Exact collection endpoints
        {"/api/posts", Router::Endpoint::AllPosts},
        {"/api/projects", Router::Endpoint::AllProjects},
        {"/api/articles", Router::Endpoint::AllArticles},

        //Parameterized endpoints
        {"/api/posts/1", Router::Endpoint::PostById, 1},
        {"/api/posts/42", Router::Endpoint::PostById, 42},
        {"/api/recentPosts/5", Router::Endpoint::RecentPosts, 5},
        {"/api/recentArticles/3", Router::Endpoint::RecentArticles, 3},
        {"/api/recentProjects/7", Router::Endpoint::RecentProjects, 7},
        {"/api/images/9", Router::Endpoint::ImageById, 9},

        //Filenames, including case-insensitive extensions
        {"/api/images/test.png", Router::Endpoint::ImageByFilename, 0, "test.png"},
        {"/api/images/photo.JPEG", Router::Endpoint::ImageByFilename, 0, "photo.JPEG"},
        {"/api/images/my-image_2.gif", Router::Endpoint::ImageByFilename, 0, "my-image_2.gif"},

        //Invalid or malicious input must not match
        {"/api/nonexistent", Router::Endpoint::NotFound},
        {"/evil/api/posts/1", Router::Endpoint::NotFound},
        {"/api/posts/1/extra", Router::Endpoint::NotFound},
        {"/api/posts/abc", Router::Endpoint::NotFound},
        {"/api/posts/-1", Router::Endpoint::NotFound},
        {"/api/posts/99999999999999999999", Router::Endpoint::NotFound}, //int overflow
        {"/api/images/file.txt", Router::Endpoint::NotFound},
        {"/api/images/../../etc/passwd", Router::Endpoint::NotFound},
        {"/api/images/sub/dir.png", Router::Endpoint::NotFound},
        {"", Router::Endpoint::NotFound},
        {"/", Router::Endpoint::NotFound}
    };

    for (const auto& tc : cases) {
        Router::Route r = Router::match(tc.uri);
        if (r.endpoint != tc.expected) {
            std::cerr << "Test Failed: URI '" << tc.uri
                      << "' matched endpoint " << static_cast<int>(r.endpoint)
                      << " but expected " << static_cast<int>(tc.expected) << std::endl;
            std::exit(1);
        }
        if (r.endpoint != Router::Endpoint::NotFound && r.intParam != tc.intParam) {
            std::cerr << "Test Failed: URI '" << tc.uri
                      << "' extracted intParam " << r.intParam
                      << " but expected " << tc.intParam << std::endl;
            std::exit(1);
        }
        if (r.strParam != tc.strParam) {
            std::cerr << "Test Failed: URI '" << tc.uri
                      << "' extracted strParam '" << r.strParam
                      << "' but expected '" << tc.strParam << "'" << std::endl;
            std::exit(1);
        }
    }
    std::cout << "test_router passed!" << std::endl;
}

int main() {
    try {
        test_toLowerCamelCase();
        test_router();
        std::cout << "\nAll unit tests passed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
