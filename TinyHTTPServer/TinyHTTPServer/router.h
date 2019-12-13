#ifndef _HEADER_ROUTER_
#define _HEADER_ROUTER_
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

struct Request;
struct Response;
class View;
using ViewPtr = std::shared_ptr<View>;

// Router ����URL��������Ӧ��View,���ڳ���ʱ�ҵ�������errorHandler
class Router
{
public:
    struct Route
    {
        std::string              url, urlRegex;
        int                      supportedMethods;
        ViewPtr                  view;
        std::vector<std::string> paramNames;
    };

    Router();

    void                      setRoute(std::string url, int supportedMethods, ViewPtr view);
    void                      removeRoute(std::string url);
    const std::vector<Route> &getAllRoutes() const;

    void setErrorHandler(int statusCode, ViewPtr view);
    void removeErrorHandler(int statusCode);

    ViewPtr resolve(Request &request, Response &response) const;
    ViewPtr getErrorHandler(int statusCode) const;

private:
    std::vector<Route>     urlMap;
    std::map<int, ViewPtr> errorMap;

    std::string getAllowedString(int supportedMethods) const;
};

#endif
