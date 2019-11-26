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
    if (getWaitingState())  // ���ͷ����ڵȴ�ȷ��״̬
        return false;

    Packet packet;
    packet.acknum   = -1;  // ���Ը��ֶ�
    packet.seqnum   = nextSeqNum;
    packet.checksum = 0;
    memcpy(packet.payload, message.data, sizeof(message.data));
    packet.checksum = pUtils->calculateCheckSum(packet);

    rbuf.put(packet);  // ���뻺����

    pUtils->printPacket("���ͷ����ͱ���", packet);
    // ����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
    pns->sendToNetworkLayer(RECEIVER, packet);

    if (baseSeqNum == nextSeqNum)
        pns->startTimer(SENDER, Configuration::TIME_OUT, baseSeqNum);  // �������ͷ���ʱ��

    nextSeqNum++;  // �������
    return true;
}

void GBNRdtSender::receive(const Packet &ackPkt)
{
    if (baseSeqNum == nextSeqNum)  // �����ȷ������Ϊ�գ���ʲô������
        return;

    // ���У����Ƿ���ȷ
    int checkSum = pUtils->calculateCheckSum(ackPkt);

    // ���У�����ȷ������ȷ������ڷ��ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ������
    if (checkSum == ackPkt.checksum && ackPkt.acknum >= baseSeqNum && ackPkt.acknum < nextSeqNum) {
        pns->stopTimer(SENDER, baseSeqNum);  // �رն�ʱ��
        std::stringstream msg;
        msg << "���ͷ���ȷ�յ�ȷ��, �ƶ���������(" << baseSeqNum << "," << nextSeqNum;

        int ackCount = ackPkt.acknum - baseSeqNum;
        baseSeqNum   = ackPkt.acknum + 1;

        msg << " -> " << baseSeqNum << "," << nextSeqNum << ")";
        pUtils->printPacket(msg.str().c_str(), ackPkt);

        for (int i = 0; i <= ackCount; i++) {
            rbuf.get();
        }

        if (baseSeqNum < nextSeqNum)  // ����ȴ�ȷ�϶����л��а���������ʱ��
            pns->startTimer(SENDER, Configuration::TIME_OUT, baseSeqNum);
    }
    else {
        pUtils->printPacket("���ͷ�û����ȷ�յ�ȷ��", rbuf.head());
    }
}

void GBNRdtSender::timeoutHandler(int seqNum)
{
    pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ȴ�ȷ�ϱ���", rbuf.head());
    pns->stopTimer(SENDER, seqNum);                                // ���ȹرն�ʱ��
    pns->startTimer(SENDER, Configuration::TIME_OUT, baseSeqNum);  // �����������ͷ���ʱ��

    // ���·������ݰ�
    for (auto &packet : rbuf) {
        pns->sendToNetworkLayer(RECEIVER, packet);
    }
}