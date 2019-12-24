#ifndef _HEADER_HTTPSERVER_
#define _HEADER_HTTPSERVER_
#pragma once

#include "request.h"
#include "response.h"
#include "router.h"
#include "syncLock.h"

#include <atomic>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>
#include <winsock2.h>

struct Connection
{
    SOCKET      socket;  // 客户端连接socket
    sockaddr_in addr;    // 客户端IPv4地址

    ~Connection();
    Connection() : socket(INVALID_SOCKET) {}
    Connection(const Connection &conn) = delete;
    Connection(Connection &&conn);

    uint16_t    port() const { return ntohs(addr.sin_port); }
    std::string ipv4_str() const { return ip_addr() + ":" + std::to_string(port()); }
    std::string ip_addr() const;
};

struct ClientInfo
{
    Connection conn;                // 客户端连接(socket与地址信息)
    Request    request;             // 请求结构
    Response   response;            // 响应结构
    int        bytesReceived;       // 单次收到的字节数
    int        totalBytesReceived;  // 总收到的字节数
    int        bytesSent;           // 单次发送的字节数
    int        totalBytesSent;      // 总发送的字节数
};

class HttpServer
{
    Connection listenConn;  // 监听连接(socket与地址信息)

    std::ostream &   logStream;   // 日志流
    SyncLock         logLock;     // 日志锁
    std::atomic_bool running;     // 运行中标志
    std::mutex       runningMtx;  // 运行互斥锁

    Router router;  // 路由模块

    // 记录当前所有客户端连接
    std::vector<ClientInfo *> clientList;
    std::mutex                clientListMtx;

    void handleConnection(Connection &&conn);  // 响应处理线程
    bool sendResponse(ClientInfo &client);     // 发送响应报文

public:
    static int  SelectListenIntervalMS;                       // 监听循环等待时长(毫秒)
    std::string serverName = "TinyHttpServer/0.1 (Windows)";  // 服务器默认名

    HttpServer(const char *host, uint16_t port, const Router &router, std::ostream &logstream);
    ~HttpServer();

    // 状态查询函数
    bool        isRunning() const { return running.load(std::memory_order_relaxed); }
    auto        getCurrentClients() const { return clientList; }
    std::string ipAddress() const { return listenConn.ipv4_str(); }

    void run();    // 监听循环
    void start();  // 在新线程启动监听循环
    void stop();   // 停止线程中的监听循环
};

#endif  // !_HEADER_HTTPSERVER_
