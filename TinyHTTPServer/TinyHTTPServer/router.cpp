#include "router.h"
#include "request.h"
#include <regex>

Router::Router() {}

void Router::setRoute(std::string url, ViewPtr view) {
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
    while (std::regex_search(url, match, urlParamSearchRe)) {
        std::string paramStr = match[0].str();

        paramStr = paramStr.substr(1, paramStr.length() - 2);
        if (paramStr.find("path:") == 0)
            paramStr = paramStr.substr(5);

        paramNames.push_back(paramStr);
        url = match.suffix();
    }

    urlMap[urlRe] = std::make_pair(view, paramNames);
}

void Router::removeRoute(std::string url) {
    urlMap.erase(url);
}

ViewPtr Router::resolve(Request& request) {
    for (auto it = urlMap.cbegin(); it != urlMap.cend(); it++) {
        std::regex urlRe(it->first);

        std::smatch match;
        if (std::regex_match(request.url, match, urlRe)) {
            for (int i = 1; i < match.size(); i++) {
                request.urlParams[it->second.second[i - 1]] = match[i].str();
            }
            return it->second.first; 
        }
    }

    return nullptr;
}