#ifndef _HEADER_HTTPSERVER_
#define _HEADER_HTTPSERVER_
#pragma once

#include "syncLock.h"
#include <cstdint>
#include <atomic>
#include <winsock2.h>


struct Connection {
    SOCKET socket;
    sockaddr addr;

    ~Connection();
    Connection() : socket(INVALID_SOCKET) {}
    Connection(Connection&& conn);
    std::string ipv4_str();
};

class HttpServer {
    SOCKET listenSocket;
    uint16_t port;
    std::ostream& logStream;
    SyncLock logLock;
    std::atomic_bool running;

    void handleConnection(Connection&& conn);

public:
    static const int MaxRequestBufferLength = 4096;

    HttpServer(uint16_t port, std::ostream& logStream);
    ~HttpServer();

    void run();
    void start();
    void stop();
};


#endif // !_HEADER_HTTPSERVER_


