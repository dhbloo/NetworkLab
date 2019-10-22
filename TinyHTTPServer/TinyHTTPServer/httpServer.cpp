#include "HttpServer.h"
#include "requestExcept.h"
#include "view.h"
#include "util.h"

#include <ws2tcpip.h>
#include <stdexcept>
#include <thread>
#include <string>
#include <cassert>

#pragma comment(lib, "Ws2_32.lib")

HttpServer::HttpServer(
    const char* host,
    uint16_t port, 
    const Router& router, 
    std::ostream& ostream
)
    : router(router), logStream(ostream) {
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
    if (int ret = getaddrinfo(host, std::to_string(port).c_str(), &hints, &result)
        || result->ai_family != AF_INET) {
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

    listenConn.addr = *(sockaddr_in*)result->ai_addr;
    freeaddrinfo(result);

    // ��ʼ����
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listenSocket);
        WSACleanup();
        throw std::runtime_error("Listen failed with error: " + std::to_string(WSAGetLastError()));
    }
    
    logStream << logLock.out << listenConn.ipv4_str() << " [Info]HTTP Server started" << logLock.endl;
}

HttpServer::~HttpServer() {
    logStream << logLock.out << listenConn.ipv4_str() << " [Info]Stopping HTTP Server..." << logLock.endl;

    // ȷ��run����û������
    if (isRunning()) stop();

    // �رռ���socket
    closesocket(listenSocket);

    // �ر������Ѿ��򿪵�socket
    {
        std::lock_guard<std::mutex> guard(clientListMtx);
        for (auto c : clientList)
            closesocket(c->conn.socket);
    }

    WSACleanup();

    // �ȴ����й����߳��˳�
    while (clientList.size()) {
        std::this_thread::yield();
    }

    logStream << logLock.out << listenConn.ipv4_str() << " [Info]HTTP Server stopped" << logLock.endl;
}

void HttpServer::handleConnection(Connection&& conn) {
    ClientInfo client{ conn, {}, {}, 0, 0, 0, 0 };
    char recvbuf[MaxRequestBufferLength];

    // �������ӵ�socket�����б�
    {
        std::lock_guard<std::mutex> guard(clientListMtx);
        clientList.push_back(&client);
    }

    assert(conn.addr.sin_family == AF_INET);
    logStream << logLock.out << conn.ipv4_str() << " [Info]Connection accepted" << logLock.endl;

    Response &response = client.response;
    int& bytesReceived = client.bytesReceived;
    bool keepAlive = true;
    do {
        std::stringstream receivedDataBuffer;
        // ��ȡ����������
        bytesReceived = recv(conn.socket, recvbuf, sizeof(recvbuf) - 1, 0);

        if (bytesReceived == 0)
            break;
        else if (bytesReceived < 0) {
            logStream << logLock.out << conn.ipv4_str() << " [Erro]Recv failed with error: "
                << WSAGetLastError() << ", closing connection" << logLock.endl;
            break;
        }

        assert(bytesReceived <= sizeof(recvbuf));
        recvbuf[bytesReceived] = '\0';
        receivedDataBuffer << recvbuf;
        client.totalBytesReceived += bytesReceived;

        Request &request = client.request;
        try {
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

            // �����Ƿ�Ϊ�־�����
            keepAlive = !(
                request.version == "HTTP/1.1" && request.lowerHeader("Connection") == "close" ||
                request.version == "HTTP/1.0" && request.lowerHeader("Connection") != "keep-alive");

            // ����Ĭ����Ӧ
            response = { "HTTP/1.1", 200 };

            // ����Router�е����û��View, û���ҵ��򶪳�404/405����
            router.resolve(request, response)->handle(request, response);

            logStream << logLock.out << conn.ipv4_str() << " [Info]Response("
                << response.statusCode << " " << response.statusInfo() << "): "
                << response.body.length() << " bytes" << logLock.endl;
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
            try {
                // ��Router��Ѱ���Ƿ��оֲ�/ȫ�ִ�����View
                ViewPtr errorView = router.getErrorHandler(response.statusCode);
                if (errorView || (errorView = router.getErrorHandler(0)))
                    errorView->handle(request, response);
                else
                    response.body = "";

                logStream << logLock.out << conn.ipv4_str() << " [Erro]Response("
                    << response.statusCode << " " << response.statusInfo() << "): "
                    << a.what() << logLock.endl;
            }
            catch (Redirect r) {
                response.statusCode = 302;
                response.headers["Location"] = r.url;

                logStream << logLock.out << conn.ipv4_str() << " [Erro]Response("
                    << response.statusCode << " " << response.statusInfo() << "): "
                    << r.what() << " Location = " << r.url << logLock.endl;
            }
            catch (Abort a) {
                response.body = "";

                logStream << logLock.out << conn.ipv4_str() << " [Erro]Response("
                    << response.statusCode << " " << response.statusInfo() << "): "
                    << " Abort in error handler!" << logLock.endl;
            }
        }

        // ����Ӧ�м���ͨ����ӦHeaders(Server, Date, Connection)
        response.headers["Server"] = serverName;
        response.headers["Date"] = Rfc1123DateTimeNow();

        // ������ǳ־�����,����Ӧͷ��ָ���Ͽ�����
        if (!keepAlive)
            response.headers["Connection"] = "close";

        // ����Ӧ����ʵ��Headers(Encoding, Length, MD5)
        if (response.headers.find("Content-Length") == response.headers.end()) {
            response.headers["Content-Encoding"] = "identity";
            response.headers["Content-Length"] = std::to_string(response.body.length());
        }
        // TODO: MD5

        // ������Ӧ
        if (!sendResponse(client))
            break;

        // �����Ƿ�Ϊ�־�����
        if (!keepAlive)
            break;

    } while (bytesReceived > 0);

    logStream << logLock.out << conn.ipv4_str() << " [Info]Connection closed" << logLock.endl;

    // �������ӵ�socket���б��г�ȥ
    {
        std::lock_guard<std::mutex> guard(clientListMtx);
        clientList.erase(std::find(clientList.begin(), clientList.end(), &client));
    }
}

bool HttpServer::sendResponse(ClientInfo& client) {
    std::string responseStr = client.response.toString();
    client.bytesSent = responseStr.size();
    client.totalBytesSent += client.bytesSent;

    int sendRet = send(client.conn.socket, responseStr.c_str(), responseStr.size(), 0);
    if (sendRet == SOCKET_ERROR) {
        logStream << logLock.out << client.conn.ipv4_str() << " [Erro]Send("
            << responseStr.length() << " bytes) failed with error: " << WSAGetLastError()
            << ", closing connection" << logLock.endl;
        return false;
    }
    return true;
}

void HttpServer::run() {
    std::lock_guard<std::mutex> guard(runningMtx);
    running.store(true, std::memory_order_relaxed);

    int nAddrLen = sizeof(Connection::addr);
    fd_set fdmaster, fdworking;
    FD_ZERO(&fdmaster);
    FD_SET(listenSocket, &fdmaster);
    const timeval tv = { 
        SelectListenIntervalMS / 1000, 
        1000 * (SelectListenIntervalMS % 1000) 
    };

    while (running.load(std::memory_order_relaxed)) {
        std::copy(&fdmaster, &fdmaster + 1, &fdworking);

        int ret = select(0, &fdworking, nullptr, nullptr, &tv);
        if (ret > 0) {
            Connection conn;
            // ���������߳�
            conn.socket = accept(listenSocket, (sockaddr*)&conn.addr, &nAddrLen);

            if (conn.socket == INVALID_SOCKET) {
                throw std::runtime_error("Error at accept(): " + std::to_string(WSAGetLastError()));
            }

            std::thread newThread(&HttpServer::handleConnection, this, std::move(conn));
            newThread.detach();
        }
        else if (ret < 0) {
            throw std::runtime_error("Error at select(): " + std::to_string(WSAGetLastError()));
        }
    }
}

void HttpServer::start() {
    if (isRunning()) 
        return;

    std::lock_guard<std::mutex> guard(runningMtx);
    std::thread listenThread(&HttpServer::run, this);
    listenThread.detach();
}

void HttpServer::stop() {
    running.store(false, std::memory_order_relaxed);
    std::lock_guard<std::mutex> guard(runningMtx);
}

Connection::~Connection() {
    if (this->socket != INVALID_SOCKET)
        closesocket(this->socket);
}

Connection::Connection(Connection&& conn) {
    std::memcpy(this, &conn, sizeof(Connection));
    conn.socket = INVALID_SOCKET;
}

std::string Connection::ip_addr() const {
    char buf[17];
    inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(buf));
    return buf;
}
