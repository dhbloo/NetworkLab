#pragma once
#include "Cfifo.h"
#include "RdtReceiver.h"

class SRRdtReceiver : public RdtReceiver
{
public:
    static const int WindowSize = 16;  // 窗口大小

private:
    int    baseSeqNum;  // 基序号
    Packet lastAckPkt;  // 上次发送的确认报文

    struct ReceivablePacket
    {
        Packet packet;
        bool   received;  // 附带bool用于指示是否接受过
    };
    CFifo<ReceivablePacket, WindowSize> rbuf;  // 环形数据包缓冲区

public:
    SRRdtReceiver();
    virtual ~SRRdtReceiver();

public:
    // 接收报文，将被NetworkService调用
    void receive(const Packet &packet);
};