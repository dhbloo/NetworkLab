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

    // ��ʼ�� Winsock
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

    // ���������׽���
    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
        WSACleanup();
        throw std::runtime_error("Error at socket(): " + std::to_string(WSAGetLastError()));
    }

    // �׽��ְ󶨶˿�
    if (bind(listenSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        freeaddrinfo(result);
        closesocket(listenSocket);
        WSACleanup();
        throw std::runtime_error("Bind failed with error: " + std::to_string(WSAGetLastError()));
    }

    freeaddrinfo(result);

    // ��ʼ����
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
        // ��ȡ����������ֱ��û����������
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
            // ����Ӧ�м���ͨ����ӦHeaders(Server, Date, Connection)
            response.headers["Server"] = serverName;
            response.headers["Date"] = Rfc1123DateTimeNow();
            if (   request.version == "HTTP/1.1" && request.getHeader("Connection") == "Close"
                || request.version == "HTTP/1.0" && request.getHeader("Connection") != "Keep-Alive")
                response.headers["Connection"] = "close";

            // �ӽ��������н���Request
            request = Request::parse(receivedDataBuffer);
            logStream << logLock.out << conn.ipv4_str() << " [Info]Request("
                << bytesReceived << " bytes) " << request.methodStr
                << request.version.substr(4) << " " << request.url << logLock.endl;

            // �����Ƿ��ǲ�֧�ֵ�HTTP����
            if (request.method == Request::UNSUPPORTED) {
                throw Abort(501, "Unsupported method " + request.methodStr);
            }

            // ����HTTP�汾
            if (request.version != "HTTP/1.0" && request.version != "HTTP/1.1") {
                throw Abort(505, "Unsupport http version " + request.version);
            }

            // ����Header��Content-Length�Ƿ���body���ȶ�Ӧ
            if (request.headers.find("Content-Length") != request.headers.end()) {
                int contentLength = std::stoi(request.headers["Content-Length"]);
                if (contentLength != request.body.length())
                    logStream << logLock.out << conn.ipv4_str() << " [Warn]Incomplete body(expect "
                    << contentLength << " bytes, actual " << request.body.length() << " bytes)"
                    << logLock.endl;
            }

            // ����Router�е����û��View, û���ҵ��򶪳�404/405����
            router.resolve(request, response)->handle(request, response);

            // ����Ӧ����ʵ��Headers(Encoding, Length, MD5)
            if (response.headers.find("Content-Encoding") == response.headers.end()) {
                response.headers["Content-Encoding"] = "identity";
                response.headers["Content-Length"] = std::to_string(response.body.length());
            }
            // TODO: MD5

        }
        // ���������ض����쳣
        catch (Redirect r) {
            response.statusCode = 302;
            logStream << logLock.out << conn.ipv4_str() << " [Info]Response("
                << response.statusCode << " " << response.statusInfo() << "): "
                << r.what() << " Location = " << r.url << logLock.endl;

            response.headers["Location"] = r.url;
        }
        // ����������ֹ�쳣
        catch (Abort a) {
            response.statusCode = a.statusCode;
            // Ĭ���������ֹ����
            response.headers["Connection"] = "close";
            logStream << logLock.out << conn.ipv4_str() << " [Erro]Response("
                << response.statusCode << " " << response.statusInfo() << "): "
                << a.what() << logLock.endl;

            // ��Router��Ѱ���Ƿ��д�����View
            ViewPtr errorView = router.getErrorHandler(response.statusCode);
            if (errorView)
                errorView->handle(request, response);
            else
                response.body = "";
        }

        // ������Ӧ
        if (!sendResponse(conn, response))
            return;

        // �����Ƿ�Ϊ�־�����
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
        // ���������߳�
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
