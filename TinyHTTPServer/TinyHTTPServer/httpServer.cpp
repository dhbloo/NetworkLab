#include "HttpServer.h"
#include "request.h"

#include <ws2tcpip.h>
#include <stdexcept>
#include <thread>
#include <string>
#include <sstream>
#include <cassert>

#pragma comment(lib, "Ws2_32.lib")

HttpServer::HttpServer(uint16_t port, std::ostream& ostream)
    : port(port), logStream(ostream) {
    WSAData wsaData;

    // 初始化 Winsock
    if (int ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        throw std::runtime_error("WSAStartup Failed: " + std::to_string(ret));
    }

    addrinfo hints, * result = nullptr;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    if (int ret = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &result)) {
        WSACleanup();
        throw std::runtime_error("Getaddrinfo failed with error: " + std::to_string(ret));
    }

    // 创建监听套接字
    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
        WSACleanup();
        throw std::runtime_error("Error at socket(): " + std::to_string(WSAGetLastError()));
    }

    // 套接字绑定端口
    if (bind(listenSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        freeaddrinfo(result);
        closesocket(listenSocket);
        WSACleanup();
        throw std::runtime_error("Bind failed with error: " + std::to_string(WSAGetLastError()));
    }

    freeaddrinfo(result);

    // 开始监听
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listenSocket);
        WSACleanup();
        throw std::runtime_error("Listen failed with error: " + std::to_string(WSAGetLastError()));
    }

    logStream << logLock.out << "HTTP Server started on port " << port << logLock.endl;
}

HttpServer::~HttpServer() {
    closesocket(listenSocket);
    WSACleanup();
}

void HttpServer::handleConnection(Connection&& conn) {
    char recvbuf[MaxRequestBufferLength];
    int bytesReceived, totalBytesReceived = 0;

    assert(conn.addr.sa_family == AF_INET);
    logStream << logLock.out << conn.ipv4_str() << " [Info]Connection accepted" << logLock.endl;

    do {
        std::stringstream receivedDataBuffer;
        // 读取缓冲区数据直到没有数据输入
        //do {
        bytesReceived = recv(conn.socket, recvbuf, sizeof(recvbuf) - 1, 0);

        if (bytesReceived == 0) {
            logStream << logLock.out << conn.ipv4_str() << " [Info]Connection closed" << logLock.endl;
            break;
        }
        else if (bytesReceived < 0) {
            logStream << logLock.out << conn.ipv4_str() << " [Erro]Recv failed with error: "
                << WSAGetLastError() << logLock.endl;

            return;
        }

        assert(bytesReceived <= sizeof(recvbuf));
        recvbuf[bytesReceived] = '\0';
        receivedDataBuffer << recvbuf;
        totalBytesReceived += bytesReceived;

        //} while (recv(conn.socket, recvbuf, sizeof(recvbuf) - 1, MSG_PEEK) > 0);

        logStream << logLock.out << conn.ipv4_str() << " [Info]Received "
            << bytesReceived << " bytes" << logLock.endl;

        // 从接受数据中解析Request
        Request request;
        try {
            request = Request::parse(receivedDataBuffer);
            logStream << logLock.out << conn.ipv4_str() << " [Info]Request(Total: "
                << bytesReceived << " bytes) " << request.methodStr << request.version.substr(4)
                << " " << request.url << logLock.endl;
        }
        catch (std::runtime_error e) {
            logStream << logLock.out << conn.ipv4_str()
                << " [Erro]Response(400 Bad Request): " << request.methodStr << logLock.endl;
            return;
        }

        // 检验是否是不支持的HTTP方法
        if (request.method == Request::UNSUPPORTED) {
            logStream << logLock.out << conn.ipv4_str()
                << " [Erro]Response(501 Not Implemented): " << request.methodStr << logLock.endl;
            return;
        }


        // 检验Header中Content-Length是否与body长度对应
        if (request.headers.find("Content-Length") != request.headers.end()) {
            int contentLength = std::stoi(request.headers["Content-Length"]);
            if (contentLength != request.body.length())
                logStream << logLock.out << conn.ipv4_str() << " [Warn]Incomplete body(expect "
                << contentLength << " bytes, actual " << request.body.length() << " bytes)"
                << logLock.endl;
        }


    } while (bytesReceived > 0);

}

void HttpServer::run() {
    int nAddrLen = sizeof(sockaddr);
    while (running.load(std::memory_order_relaxed)) {
        Connection conn;
        // 阻塞监听线程
        conn.socket = accept(listenSocket, &conn.addr, &nAddrLen);

        if (conn.socket == INVALID_SOCKET) {
            throw std::runtime_error("Error at accept(): " + std::to_string(WSAGetLastError()));
        }

        std::thread newThread(&HttpServer::handleConnection, this, std::move(conn));
        newThread.detach();
    }
}

void HttpServer::start() {
    running.store(true, std::memory_order_relaxed);
    std::thread listenThread(&HttpServer::run, this);
    listenThread.detach();
}

void HttpServer::stop() {
    running.store(false, std::memory_order_relaxed);
}

Connection::~Connection() {
    if (this->socket != INVALID_SOCKET)
        closesocket(this->socket);
}

Connection::Connection(Connection&& conn) {
    std::memcpy(this, &conn, sizeof(Connection));
    conn.socket = INVALID_SOCKET;
}

std::string Connection::ipv4_str() {
    char buf[17];
    inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    return buf;
}
