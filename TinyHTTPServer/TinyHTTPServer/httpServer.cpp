#include "HttpServer.h"
#include "request.h"
#include "requestExcept.h"
#include "response.h"
#include "view.h"
#include "util.h"

#include <ws2tcpip.h>
#include <stdexcept>
#include <thread>
#include <string>
#include <sstream>
#include <cassert>

#pragma comment(lib, "Ws2_32.lib")

HttpServer::HttpServer(uint16_t port, const Router& router, std::ostream& ostream)
    : port(port), router(router), logStream(ostream) {
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

    Response response{ "HTTP/1.1", 200 };
    do {
        std::stringstream receivedDataBuffer;
        // 读取缓冲区数据直到没有数据输入
        //do {
        bytesReceived = recv(conn.socket, recvbuf, sizeof(recvbuf) - 1, 0);

        if (bytesReceived == 0)
            break;
        else if (bytesReceived < 0) {
            logStream << logLock.out << conn.ipv4_str() << " [Erro]Recv failed with error: "
                << WSAGetLastError() << ", closing connection" << logLock.endl;
            return;
        }

        assert(bytesReceived <= sizeof(recvbuf));
        recvbuf[bytesReceived] = '\0';
        receivedDataBuffer << recvbuf;
        totalBytesReceived += bytesReceived;

        //} while (recv(conn.socket, recvbuf, sizeof(recvbuf) - 1, MSG_PEEK) > 0);

        Request request;
        try {
            // 在响应中加入通用响应Headers(Server, Date, Connection)
            response.headers["Server"] = serverName;
            response.headers["Date"] = Rfc1123DateTimeNow();
            if (   request.version == "HTTP/1.1" && request.getHeader("Connection") == "Close"
                || request.version == "HTTP/1.0" && request.getHeader("Connection") != "Keep-Alive")
                response.headers["Connection"] = "close";

            // 从接受数据中解析Request
            request = Request::parse(receivedDataBuffer);
            logStream << logLock.out << conn.ipv4_str() << " [Info]Request("
                << bytesReceived << " bytes) " << request.methodStr
                << request.version.substr(4) << " " << request.url << logLock.endl;

            // 检验是否是不支持的HTTP方法
            if (request.method == Request::UNSUPPORTED) {
                throw Abort(501, "Unsupported method " + request.methodStr);
            }

            // 检验HTTP版本
            if (request.version != "HTTP/1.0" && request.version != "HTTP/1.1") {
                throw Abort(505, "Unsupport http version " + request.version);
            }

            // 检验Header中Content-Length是否与body长度对应
            if (request.headers.find("Content-Length") != request.headers.end()) {
                int contentLength = std::stoi(request.headers["Content-Length"]);
                if (contentLength != request.body.length())
                    logStream << logLock.out << conn.ipv4_str() << " [Warn]Incomplete body(expect "
                    << contentLength << " bytes, actual " << request.body.length() << " bytes)"
                    << logLock.endl;
            }

            // 根据Router中的配置获得View, 没有找到则丢出404/405错误
            router.resolve(request, response)->handle(request, response);

            // 给响应加入实体Headers(Encoding, Length, MD5)
            if (response.headers.find("Content-Encoding") == response.headers.end()) {
                response.headers["Content-Encoding"] = "identity";
                response.headers["Content-Length"] = std::to_string(response.body.length());
            }
            // TODO: MD5

        }
        // 处理请求重定向异常
        catch (Redirect r) {
            response.statusCode = 302;
            logStream << logLock.out << conn.ipv4_str() << " [Info]Response("
                << response.statusCode << " " << response.statusInfo() << "): "
                << r.what() << " Location = " << r.url << logLock.endl;

            response.headers["Location"] = r.url;
        }
        // 处理请求终止异常
        catch (Abort a) {
            response.statusCode = a.statusCode;
            // 默认情况下终止连接
            response.headers["Connection"] = "close";
            logStream << logLock.out << conn.ipv4_str() << " [Erro]Response("
                << response.statusCode << " " << response.statusInfo() << "): "
                << a.what() << logLock.endl;

            // 在Router中寻找是否有错误处理View
            ViewPtr errorView = router.getErrorHandler(response.statusCode);
            if (errorView)
                errorView->handle(request, response);
            else
                response.body = "";
        }

        // 发送响应
        if (!sendResponse(conn, response))
            return;

        // 检验是否为持久链接
        auto connHeaderIt = response.headers.find("Connection");
        if (connHeaderIt != response.headers.end() &&
            connHeaderIt->second == "close")
            break;

    } while (bytesReceived > 0);

    logStream << logLock.out << conn.ipv4_str() << " [Info]Connection closed" << logLock.endl;
}

bool HttpServer::sendResponse(const Connection& conn, Response& response) {
    std::string responseStr = response.toString();

    int sendRet = send(conn.socket, responseStr.c_str(), responseStr.size() + 1, 0);
    if (sendRet == SOCKET_ERROR) {
        logStream << logLock.out << conn.ipv4_str() << " [Erro]Send(" << responseStr.length()
            << " bytes) failed with error: " << WSAGetLastError() << ", closing connection"
            << logLock.endl;
        return false;
    }
    return true;
}

void HttpServer::run() {
    int nAddrLen = sizeof(sockaddr);
    running.store(true, std::memory_order_relaxed);
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

std::string Connection::ipv4_str() const {
    char buf[17];
    inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    return buf;
}
