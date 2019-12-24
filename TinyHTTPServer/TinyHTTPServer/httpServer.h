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
    SOCKET      socket;  // �ͻ�������socket
    sockaddr_in addr;    // �ͻ���IPv4��ַ

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
    Connection conn;                // �ͻ�������(socket���ַ��Ϣ)
    Request    request;             // ����ṹ
    Response   response;            // ��Ӧ�ṹ
    int        bytesReceived;       // �����յ����ֽ���
    int        totalBytesReceived;  // ���յ����ֽ���
    int        bytesSent;           // ���η��͵��ֽ���
    int        totalBytesSent;      // �ܷ��͵��ֽ���
};

class HttpServer
{
    Connection listenConn;  // ��������(socket���ַ��Ϣ)

    std::ostream &   logStream;   // ��־��
    SyncLock         logLock;     // ��־��
    std::atomic_bool running;     // �����б�־
    std::mutex       runningMtx;  // ���л�����

    Router router;  // ·��ģ��

    // ��¼��ǰ���пͻ�������
    std::vector<ClientInfo *> clientList;
    std::mutex                clientListMtx;

    void handleConnection(Connection &&conn);  // ��Ӧ�����߳�
    bool sendResponse(ClientInfo &client);     // ������Ӧ����

public:
    static int  SelectListenIntervalMS;                       // ����ѭ���ȴ�ʱ��(����)
    std::string serverName = "TinyHttpServer/0.1 (Windows)";  // ������Ĭ����

    HttpServer(const char *host, uint16_t port, const Router &router, std::ostream &logstream);
    ~HttpServer();

    // ״̬��ѯ����
    bool        isRunning() const { return running.load(std::memory_order_relaxed); }
    auto        getCurrentClients() const { return clientList; }
    std::string ipAddress() const { return listenConn.ipv4_str(); }

    void run();    // ����ѭ��
    void start();  // �����߳���������ѭ��
    void stop();   // ֹͣ�߳��еļ���ѭ��
};

#endif  // !_HEADER_HTTPSERVER_
