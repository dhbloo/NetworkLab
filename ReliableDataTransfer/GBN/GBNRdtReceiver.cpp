#include "GBNRdtReceiver.h"

#include "Common.h"
#include "Global.h"

GBNRdtReceiver::GBNRdtReceiver() : expectSeqNumRcvd(0)
{
    // 初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
    lastAckPkt.acknum   = -1;
    lastAckPkt.seqnum   = -1;  // 忽略该字段
    lastAckPkt.checksum = 0;
    std::fill_n(lastAckPkt.payload, Configuration::PAYLOAD_SIZE, '.');
    lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

GBNRdtReceiver::~GBNRdtReceiver() {}

void GBNRdtReceiver::receive(const Packet &packet)
{
    // 计算校验和
    int checkSum = pUtils->calculateCheckSum(packet);

    // 如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
    if (checkSum == packet.checksum && expectSeqNumRcvd == packet.seqnum) {
        pUtils->printPacket("接收方正确收到发送方的报文", packet);

        // 取出Message，向上递交给应用层
        Message msg;
        memcpy(msg.data, packet.payload, sizeof(packet.payload));
        pns->delivertoAppLayer(RECEIVER, msg);

        // 发送Ack包, 确认序号等于收到的报文序号
        lastAckPkt.acknum   = packet.seqnum;
        lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
        pUtils->printPacket("接收方发送确认报文", lastAckPkt);

        expectSeqNumRcvd++;  // 接收序号增加1
    }
    else {
        if (checkSum != packet.checksum) {
            pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
        }
        else {
            pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
        }
        pUtils->printPacket("接收方重新发送上次的确认报文", lastAckPkt);
    }

    // 调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文
    pns->sendToNetworkLayer(SENDER, lastAckPkt);
}
