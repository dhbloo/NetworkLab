#ifndef _HEADER_ROUTER_
#define _HEADER_ROUTER_
#pragma once

#include <map>
#include <string>
#include <memory>
#include <vector>

class Request;
class View;
//using ViewPtr = std::shared_ptr<View>;
using ViewPtr = int;

class Router {
    std::map<std::string, std::pair<ViewPtr, std::vector<std::string>>> urlMap;
    ViewPtr fallbackView;

public:
    Router();

    void setRoute(std::string url, ViewPtr view);
    void removeRoute(std::string url);
    ViewPtr resolve(Request& request);
};

#endif

