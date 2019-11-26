#include "SRRdtSender.h"

#include "Common.h"
#include "Global.h"

SRRdtSender::SRRdtSender() : baseSeqNum(0), nextSeqNum(0) {}

SRRdtSender::~SRRdtSender() {}

bool SRRdtSender::getWaitingState()
{
    return rbuf.full();
}

bool SRRdtSender::send(const Message &message)
{
    if (getWaitingState())  // 发送方处于等待确认状态
        return false;

    Packet packet;
    packet.acknum   = -1;  // 忽略该字段
    packet.seqnum   = nextSeqNum;
    packet.checksum = 0;
    memcpy(packet.payload, message.data, sizeof(message.data));
    packet.checksum = pUtils->calculateCheckSum(packet);

    rbuf.put({packet, false});  // 未确认包加入缓冲区

    pUtils->printPacket("发送方发送报文", packet);
    // 调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
    pns->sendToNetworkLayer(RECEIVER, packet);
    // 启动分组对应的定时器
    pns->startTimer(SENDER, Configuration::TIME_OUT, nextSeqNum);

    nextSeqNum++;  // 序号自增
    return true;
}

void SRRdtSender::receive(const Packet &ackPkt)
{
    if (baseSeqNum == nextSeqNum)  // 如果待确认序列为空，则什么都不做
        return;

    // 检查校验和是否正确
    int checkSum = pUtils->calculateCheckSum(ackPkt);

    // 如果校验和正确，并且确认序号在发送方已发送并等待确认的数据包序号中
    if (checkSum == ackPkt.checksum && ackPkt.acknum >= baseSeqNum && ackPkt.acknum < nextSeqNum) {
        pns->stopTimer(SENDER, ackPkt.acknum);  // 关闭数据包定时器

        unsigned long offset = ackPkt.acknum - baseSeqNum;
        rbuf[offset].ack   = true;  // 将acknum对应的包标记为已接受

        std::stringstream msg;
        msg << "接收方正确收到确认, 移动滑动窗口(" << baseSeqNum << "," << nextSeqNum;

        if (offset == 0) {                              // 如果收到的包序号为base
            while (!rbuf.empty() && rbuf.head().ack) {  // 基序号移动到未确认处开头
                rbuf.get();
                baseSeqNum++;  // 基序号向后移动，直到第一个未确认的包
            }
        }

        msg << " -> " << baseSeqNum << "," << nextSeqNum << ")";
        pUtils->printPacket(msg.str().c_str(), ackPkt);
    }
    else {
        pUtils->printPacket("发送方没有正确收到确认", rbuf.head().packet);
    }
}

void SRRdtSender::timeoutHandler(int seqNum)
{
    unsigned long offset = seqNum - baseSeqNum;
    pUtils->printPacket("发送方定时器时间到，重发等待确认报文", rbuf[offset].packet);

    pns->stopTimer(SENDER, seqNum);                            // 首先关闭定时器
    pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);  // 重新启动数据包定时器
    pns->sendToNetworkLayer(RECEIVER, rbuf[offset].packet);    // 重新发送数据包
}