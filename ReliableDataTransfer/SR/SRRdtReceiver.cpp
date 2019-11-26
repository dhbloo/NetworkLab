#include "SRRdtReceiver.h"

#include "Common.h"
#include "Global.h"

#include <algorithm>

SRRdtReceiver::SRRdtReceiver() : baseSeqNum(0)
{
    // 初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
    lastAckPkt.acknum   = -1;
    lastAckPkt.seqnum   = -1;  // 忽略该字段
    lastAckPkt.checksum = 0;
    std::fill_n(lastAckPkt.payload, Configuration::PAYLOAD_SIZE, '.');
    lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);

    while (!rbuf.full())  // 初始化接收环形缓冲区
        rbuf.put({{}, false});
}

SRRdtReceiver::~SRRdtReceiver() {}

void SRRdtReceiver::receive(const Packet &packet)
{
    // 计算校验和
    int checkSum = pUtils->calculateCheckSum(packet);

    // 计算出序号在接收区的偏移量
    unsigned long offset = packet.seqnum - baseSeqNum;

    // 如果校验和正确，同时收到报文的序号在可接收范围内
    if (checkSum == packet.checksum && offset < WindowSize) {
        if (!rbuf[offset].received) {  // 如果该包还没接收过
            rbuf[offset].packet   = packet;
            rbuf[offset].received = true;  // 将数据包标记为已接收
        }

        // 发送Ack包, 确认序号等于收到的报文序号
        lastAckPkt.acknum   = packet.seqnum;
        lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
        pUtils->printPacket("接收方发送确认报文", lastAckPkt);
        // 调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文
        pns->sendToNetworkLayer(SENDER, lastAckPkt);

        std::stringstream msg;
        msg << "接收方正确收到发送方的报文, 移动滑动窗口(" << baseSeqNum << "," << baseSeqNum + rbuf.capacity();

        if (offset == 0) {                                   // 如果收到的包序号为base
            while (!rbuf.empty() && rbuf.head().received) {  // 将基序号开始的已确认报递交
                // 取出Message，向上递交给应用层
                Message msg;
                memcpy(msg.data, rbuf.get().packet.payload, sizeof(packet.payload));
                pns->delivertoAppLayer(RECEIVER, msg);

                baseSeqNum++;           // 已接收序号加1
                rbuf.put({{}, false});  // 接收区加入一个空位
            }
        }

        msg << " -> " << baseSeqNum << "," << baseSeqNum + rbuf.capacity() << ")\n";
        pUtils->printPacket(msg.str().c_str(), packet);
    }
    else {
        if (checkSum != packet.checksum) {
            pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
        }
        else {
            if (packet.seqnum - baseSeqNum <= WindowSize) {  // 收到已确认范围内的包
                // 发送Ack包, 确认序号等于收到的报文序号
                lastAckPkt.acknum   = packet.seqnum;
                lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
                pUtils->printPacket("接收方收到已接收的报文,重发ACK", lastAckPkt);
                pns->sendToNetworkLayer(SENDER, lastAckPkt);
            }
            else
                pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
        }
        pUtils->printPacket("接收方重新发送上次的确认报文", lastAckPkt);
    }
}
