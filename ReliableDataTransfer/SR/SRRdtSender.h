#pragma once
#include "Cfifo.h"
#include "RdtSender.h"

class SRRdtSender : public RdtSender
{
public:
    static const int WindowSize = 16;  // 窗口大小

private:
    int baseSeqNum;  // 基序号
    int nextSeqNum;  // 下一序号

    struct CheckablePacket
    {
        Packet packet;
        bool   ack;  // 附带bool用于指示是否确认过
    };
    CFifo<CheckablePacket, WindowSize> rbuf;  // 环形数据包缓冲区

public:
    // 发送应用层下来的Message，由NetworkService调用,如果发送方成功地将Message发送到网络层，返回true;
    // 如果因为发送方处于等待确认状态或发送窗口已满而拒绝发送Message，则返回false
    bool send(const Message &message);
    // 接受确认Ack，将被NetworkService调用
    void receive(const Packet &ackPkt);
    // Timeout handler，将被NetworkService调用
    void timeoutHandler(int seqNum);
    // 返回RdtSender是否处于等待状态，如果发送方正等待确认或者发送窗口已满，返回true
    bool getWaitingState();

public:
    SRRdtSender();
    virtual ~SRRdtSender();
};