#pragma once
#include "RdtReceiver.h"

class GBNTcpRdtReceiver : public RdtReceiver
{
private:
    int    expectSeqNumRcvd;  // 期待收到的下一个报文序号
    Packet lastAckPkt;        // 上次发送的确认报文

public:
    GBNTcpRdtReceiver();
    virtual ~GBNTcpRdtReceiver();

public:
    // 接收报文，将被NetworkService调用
    void receive(const Packet &packet);
};