#include "router.h"
#include "request.h"
#include <regex>
#include <algorithm>

Router::Router() {}

void Router::setRoute(std::string url, ViewPtr view, int supportedMethods) {
    const std::regex urlParamRe("<[^<>/]+>");
    const std::regex urlParamPathRe("<path:[^<>/]+>");
    const std::regex urlParamSearchRe("<[^<>/]+>|<path:[^<>/]+>");

    std::string urlRe(
        std::regex_replace(
            std::regex_replace(url, urlParamPathRe, "(\\S+)")
            , urlParamRe, "([^/]+)")
    );

    std::smatch match;
    std::vector<std::string> paramNames;
    std::string urlSuffix = url;
    while (std::regex_search(urlSuffix, match, urlParamSearchRe)) {
        std::string paramStr = match[0].str();

        paramStr = paramStr.substr(1, paramStr.length() - 2);
        if (paramStr.find("path:") == 0)
            paramStr = paramStr.substr(5);

        paramNames.push_back(paramStr);
        urlSuffix = match.suffix();
    }

    urlMap.push_back({ url, urlRe, supportedMethods, view, paramNames });
}

void Router::removeRoute(std::string url) {
    urlMap.erase(std::remove_if(
        urlMap.begin(), urlMap.end(),
        [&url](const Route& route) { return route.url == url; }
    ));
}

const std::vector<Router::Route>& Router::getAllRoutes() const {
    return urlMap;
}

ViewPtr Router::resolve(Request& request) const {
    for (auto r = urlMap.cbegin(); r != urlMap.cend(); r++) {
        if (!(request.method & r->supportedMethods))
            continue;

        std::regex urlRe(r->urlRegex);
        std::smatch match;
        if (std::regex_match(request.url, match, urlRe)) {
            for (int i = 1; i < match.size(); i++) {
                request.urlParams[r->paramNames[i - 1]] = match[i].str();
            }
            return r->view;
        }
    }

    return nullptr;
}