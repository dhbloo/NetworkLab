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
    std::map<std::string, std::pair<ViewPtr, std::vector<std::string>>> urlMap;

public:
    Router();

    void setRoute(std::string url, ViewPtr view);
    void removeRoute(std::string url);
    ViewPtr resolve(Request& request);
};

#endif

