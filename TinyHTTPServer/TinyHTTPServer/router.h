#ifndef _HEADER_ROUTER_
#define _HEADER_ROUTER_
#pragma once

#include <map>
#include <string>
#include <memory>
#include <vector>

class Request;
class View;
using ViewPtr = std::shared_ptr<View>;

class Router {
public:
    struct Route {
        std::string url, urlRegex;
        int supportedMethods;
        ViewPtr view;
        std::vector<std::string> paramNames;
    };

    Router();

    void setRoute(std::string url, ViewPtr view, int supportedMethods);
    void removeRoute(std::string url);
    const std::vector<Route>& getAllRoutes() const;
    ViewPtr resolve(Request& request) const;

private:
    std::vector<Route> urlMap;
};

#endif

