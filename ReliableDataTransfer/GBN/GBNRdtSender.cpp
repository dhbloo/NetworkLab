#include "GBNRdtSender.h"

#include "Common.h"
#include "Global.h"

GBNRdtSender::GBNRdtSender() : baseSeqNum(0), nextSeqNum(0) {}

GBNRdtSender::~GBNRdtSender() {}

bool GBNRdtSender::getWaitingState()
{
    return rbuf.full();
}

bool GBNRdtSender::send(const Message &message)
{
    if (getWaitingState())  // 发送方处于等待确认状态
        return false;

    Packet packet;
    packet.acknum   = -1;  // 忽略该字段
    packet.seqnum   = nextSeqNum;
    packet.checksum = 0;
    memcpy(packet.payload, message.data, sizeof(message.data));
    packet.checksum = pUtils->calculateCheckSum(packet);

    rbuf.put(packet);  // 加入缓冲区

    pUtils->printPacket("发送方发送报文", packet);
    // 调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
    pns->sendToNetworkLayer(RECEIVER, packet);

    if (baseSeqNum == nextSeqNum)
        pns->startTimer(SENDER, Configuration::TIME_OUT, baseSeqNum);  // 启动发送方定时器

    nextSeqNum++;  // 序号自增
    return true;
}

void GBNRdtSender::receive(const Packet &ackPkt)
{
    if (baseSeqNum == nextSeqNum)  // 如果待确认序列为空，则什么都不做
        return;

    // 检查校验和是否正确
    int checkSum = pUtils->calculateCheckSum(ackPkt);

    // 如果校验和正确，并且确认序号在发送方已发送并等待确认的数据包序号中
    if (checkSum == ackPkt.checksum && ackPkt.acknum >= baseSeqNum && ackPkt.acknum < nextSeqNum) {
        pns->stopTimer(SENDER, baseSeqNum);  // 关闭定时器
        std::stringstream msg;
        msg << "发送方正确收到确认, 移动滑动窗口(" << baseSeqNum << "," << nextSeqNum;

        int ackCount = ackPkt.acknum - baseSeqNum;
        baseSeqNum   = ackPkt.acknum + 1;

        msg << " -> " << baseSeqNum << "," << nextSeqNum << ")";
        pUtils->printPacket(msg.str().c_str(), ackPkt);

        for (int i = 0; i <= ackCount; i++) {
            rbuf.get();
        }

        if (baseSeqNum < nextSeqNum)  // 如果等待确认队列中还有包，启动定时器
            pns->startTimer(SENDER, Configuration::TIME_OUT, baseSeqNum);
    }
    else {
        pUtils->printPacket("发送方没有正确收到确认", rbuf.head());
    }
}

void GBNRdtSender::timeoutHandler(int seqNum)
{
    pUtils->printPacket("发送方定时器时间到，重发等待确认报文", rbuf.head());
    pns->stopTimer(SENDER, seqNum);                                // 首先关闭定时器
    pns->startTimer(SENDER, Configuration::TIME_OUT, baseSeqNum);  // 重新启动发送方定时器

    // 重新发送数据包
    for (auto &packet : rbuf) {
        pns->sendToNetworkLayer(RECEIVER, packet);
    }
}