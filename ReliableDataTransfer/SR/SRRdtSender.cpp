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
    if (getWaitingState())  // ���ͷ����ڵȴ�ȷ��״̬
        return false;

    Packet packet;
    packet.acknum   = -1;  // ���Ը��ֶ�
    packet.seqnum   = nextSeqNum;
    packet.checksum = 0;
    memcpy(packet.payload, message.data, sizeof(message.data));
    packet.checksum = pUtils->calculateCheckSum(packet);

    rbuf.put({packet, false});  // δȷ�ϰ����뻺����

    pUtils->printPacket("���ͷ����ͱ���", packet);
    // ����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
    pns->sendToNetworkLayer(RECEIVER, packet);
    // ���������Ӧ�Ķ�ʱ��
    pns->startTimer(SENDER, Configuration::TIME_OUT, nextSeqNum);

    nextSeqNum++;  // �������
    return true;
}

void SRRdtSender::receive(const Packet &ackPkt)
{
    if (baseSeqNum == nextSeqNum)  // �����ȷ������Ϊ�գ���ʲô������
        return;

    // ���У����Ƿ���ȷ
    int checkSum = pUtils->calculateCheckSum(ackPkt);

    // ���У�����ȷ������ȷ������ڷ��ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ������
    if (checkSum == ackPkt.checksum && ackPkt.acknum >= baseSeqNum && ackPkt.acknum < nextSeqNum) {
        pns->stopTimer(SENDER, ackPkt.acknum);  // �ر����ݰ���ʱ��

        unsigned long offset = ackPkt.acknum - baseSeqNum;
        rbuf[offset].ack   = true;  // ��acknum��Ӧ�İ����Ϊ�ѽ���

        std::stringstream msg;
        msg << "���շ���ȷ�յ�ȷ��, �ƶ���������(" << baseSeqNum << "," << nextSeqNum;

        if (offset == 0) {                              // ����յ��İ����Ϊbase
            while (!rbuf.empty() && rbuf.head().ack) {  // ������ƶ���δȷ�ϴ���ͷ
                rbuf.get();
                baseSeqNum++;  // ���������ƶ���ֱ����һ��δȷ�ϵİ�
            }
        }

        msg << " -> " << baseSeqNum << "," << nextSeqNum << ")";
        pUtils->printPacket(msg.str().c_str(), ackPkt);
    }
    else {
        pUtils->printPacket("���ͷ�û����ȷ�յ�ȷ��", rbuf.head().packet);
    }
}

void SRRdtSender::timeoutHandler(int seqNum)
{
    unsigned long offset = seqNum - baseSeqNum;
    pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ȴ�ȷ�ϱ���", rbuf[offset].packet);

    pns->stopTimer(SENDER, seqNum);                            // ���ȹرն�ʱ��
    pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);  // �����������ݰ���ʱ��
    pns->sendToNetworkLayer(RECEIVER, rbuf[offset].packet);    // ���·������ݰ�
}