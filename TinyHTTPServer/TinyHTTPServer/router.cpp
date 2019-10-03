#include "router.h"
#include "request.h"
#include "requestExcept.h"
#include "response.h"
#include <regex>
#include <algorithm>

Router::Router() {}

void Router::setRoute(std::string url, int supportedMethods, ViewPtr view) {
    const std::regex urlParamRe("<[^<>/]+>");
    const std::regex urlParamPathRe("<path:[^<>/]+>");
    const std::regex urlParamSearchRe("<[^<>/]+>|<path:[^<>/]+>");

    std::string urlRe(
        std::regex_replace(
            std::regex_replace(url, urlParamPathRe, "(\\S*)")
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

void Router::setErrorHandler(int statusCode, ViewPtr view) {
    errorMap[statusCode] = view;
}

void Router::removeErrorHandler(int statusCode) {
    errorMap.erase(statusCode);
}

ViewPtr Router::resolve(Request& request, Response& response) const {
    bool routeFinded = false;
    int routeSupportedMethods;

    for (auto r = urlMap.cbegin(); r != urlMap.cend(); r++) {
        std::regex urlRe(r->urlRegex);
        std::smatch match;
        if (std::regex_match(request.url, match, urlRe)) {
            if (!(request.method & r->supportedMethods)) {
                if (!routeFinded) {
                    routeFinded = true;
                    routeSupportedMethods = r->supportedMethods;
                }
                continue;
            }

            for (int i = 1; i < match.size(); i++) {
                request.urlParams[r->paramNames[i - 1]] = match[i].str();
            }
            return r->view;
        }
    }

    if (routeFinded) {
        std::string allowed = getAllowedString(routeSupportedMethods);
        if (!allowed.empty())
            response.headers["Allow"] = allowed;

        throw Abort(405, "Method " + getAllowedString(request.method) +
            std::string(" not allowed for ") + request.url);
    }

    throw Abort(404, "Url not found " + request.url);
    return nullptr;
}

ViewPtr Router::getErrorHandler(int statusCode) const {
    auto it = errorMap.find(statusCode);
    return it != errorMap.end() ? it->second : nullptr;
}

std::string Router::getAllowedString(int methods) const {
    std::string allowed;
    if (methods & Request::GET) allowed.append("GET, ");
    if (methods & Request::HEAD) allowed.append("HEAD, ");
    if (methods & Request::POST) allowed.append("POST, ");
    if (methods & Request::PUT) allowed.append("PUT, ");
    if (methods & Request::DEL) allowed.append("DELETE, ");
    if (methods & Request::OPTIONS) allowed.append("OPTIONS, ");

    if (!allowed.empty()) {
        allowed.pop_back();
        allowed.pop_back();
    }
    return allowed;
}
