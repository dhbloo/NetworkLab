#ifndef _HEADER_HTTPSERVER_
#define _HEADER_HTTPSERVER_
#pragma once

#include "syncLock.h"
#include "router.h"
#include "request.h"
#include "response.h"
#include <cstdint>
#include <atomic>
#include <string>
#include <ostream>
#include <sstream>
#include <winsock2.h>

class Response;

struct Connection {
    SOCKET socket;
    sockaddr_in addr;

    ~Connection();
    Connection() : socket(INVALID_SOCKET) {}
    Connection(Connection&& conn);
    Connection(SOCKET socket, sockaddr_in addr) : socket(socket), addr(addr) {}

    uint16_t port() const { return ntohs(addr.sin_port); }
    std::string ipv4_str() const { return ip_addr() + ":" + std::to_string(port()); }
    std::string ip_addr() const;
};

struct ClientInfo {
    Connection& conn;
    Request request;
    Response response;
    std::stringstream receivedDataBuffer;
    int bytesReceived;
    int totalBytesReceived;
    int bytesSent;
    int totalBytesSent;
};


class HttpServer {
    SOCKET listenSocket;
    Connection listenConn;

    std::ostream& logStream;
    SyncLock logLock;
    std::atomic_bool running;
    std::mutex runningMtx;

    Router router;

    // 记录当前所有客户端连接
    std::vector<const ClientInfo*> clientList;
    std::mutex clientListMtx;

    void handleConnection(Connection&& conn);
    bool sendResponse(ClientInfo& client);

public:
    static const int MaxRequestBufferLength = 8192;
    static const int SelectListenIntervalMS = 500;
    std::string serverName = "TinyHttpServer/0.1 (Windows)";

    HttpServer(const char* host, uint16_t port, const Router& router, std::ostream& ostream);
    ~HttpServer();

    bool isRunning() const { return running.load(std::memory_order_relaxed); }
    const std::vector<const ClientInfo*> getCurrentClients() const { return clientList; }
    std::string ipAddress() const { return listenConn.ipv4_str(); }

    void run();
    void start();
    void stop();
};


#endif // !_HEADER_HTTPSERVER_


