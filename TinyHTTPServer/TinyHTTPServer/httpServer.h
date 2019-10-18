#ifndef _HEADER_HTTPSERVER_
#define _HEADER_HTTPSERVER_
#pragma once

#include "syncLock.h"
#include "router.h"
#include <cstdint>
#include <atomic>
#include <string>
#include <ostream>
#include <winsock2.h>

class Response;

struct Connection {
    SOCKET socket;
    sockaddr_in addr;

    ~Connection();
    Connection() : socket(INVALID_SOCKET) {}
    Connection(Connection&& conn);
    std::string ipv4_str() const;
};


class HttpServer {
    SOCKET listenSocket;
    uint16_t port;
    std::ostream& logStream;
    SyncLock logLock;
    std::atomic_bool running;

    Router router;

    void handleConnection(Connection&& conn);
    bool sendResponse(const Connection& conn, Response& response);

public:
    static const int MaxRequestBufferLength = 8192;
    std::string serverName = "TinyHttpServer/0.1 (Windows)";

    HttpServer(const char* ip, uint16_t port, const Router& router, std::ostream& ostream);
    ~HttpServer();

    bool isRunning() { return running.load(std::memory_order_relaxed); }

    void run();
    void start();
    void stop();
};


#endif // !_HEADER_HTTPSERVER_


