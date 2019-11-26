#include "GBNTcpRdtSender.h"

#include "Common.h"
#include "Global.h"

GBNTcpRdtSender::GBNTcpRdtSender() : baseSeqNum(0), nextSeqNum(0), duplicateCnt(0) {}

GBNTcpRdtSender::~GBNTcpRdtSender() {}

bool GBNTcpRdtSender::getWaitingState()
{
    return rbuf.full();
}

bool GBNTcpRdtSender::send(const Message &message)
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

void GBNTcpRdtSender::receive(const Packet &ackPkt)
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
        duplicateCnt = 0;

        msg << " -> " << baseSeqNum << "," << nextSeqNum << ")";
        pUtils->printPacket(msg.str().c_str(), ackPkt);

        for (int i = 0; i <= ackCount; i++) {
            rbuf.get();
        }

        if (baseSeqNum < nextSeqNum)  // ����ȴ�ȷ�϶����л��а���������ʱ��
            pns->startTimer(SENDER, Configuration::TIME_OUT, baseSeqNum);
    }
    else {
        if (checkSum != ackPkt.checksum) {
            pUtils->printPacket("���ͷ�û����ȷ�յ�ȷ��, ����У�����", rbuf.head());
        }
        else if (++duplicateCnt >= DuplicateCntMax) {  // �������ACK���ﵽ�����ش�����
            duplicateCnt = 0;                          // ����ACK��������

            pns->stopTimer(SENDER, baseSeqNum);                            // ���ȹرն�ʱ��
            pns->startTimer(SENDER, Configuration::TIME_OUT, baseSeqNum);  // �����������ͷ���ʱ��

            pUtils->printPacket("���ͷ�û����ȷ�յ�ȷ��, ���������ش�", rbuf.head());
            pns->sendToNetworkLayer(RECEIVER, rbuf.head());  // �����ش�
        }
        else {
            pUtils->printPacket("���ͷ�û����ȷ�յ�ȷ��, ������Ų���", rbuf.head());
        }
    }
}

void GBNTcpRdtSender::timeoutHandler(int seqNum)
{
    pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ȴ�ȷ�ϱ���", rbuf.head());
    pns->stopTimer(SENDER, seqNum);                                // ���ȹرն�ʱ��
    pns->startTimer(SENDER, Configuration::TIME_OUT, baseSeqNum);  // �����������ͷ���ʱ��

    pns->sendToNetworkLayer(RECEIVER, rbuf.head());  // ֻ���·�������δ��ȷ�ϵ����ݰ�
}