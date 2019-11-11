#include "GBNRdtReceiver.h"

#include "Common.h"
#include "Global.h"

GBNRdtReceiver::GBNRdtReceiver() : expectSeqNumRcvd(0)
{
    // ��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
    lastAckPkt.acknum   = -1;
    lastAckPkt.seqnum   = -1;  // ���Ը��ֶ�
    lastAckPkt.checksum = 0;
    std::fill_n(lastAckPkt.payload, Configuration::PAYLOAD_SIZE, '.');
    lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

GBNRdtReceiver::~GBNRdtReceiver() {}

void GBNRdtReceiver::receive(const Packet &packet)
{
    // ����У���
    int checkSum = pUtils->calculateCheckSum(packet);

    // ���У�����ȷ��ͬʱ�յ����ĵ���ŵ��ڽ��շ��ڴ��յ��ı������һ��
    if (checkSum == packet.checksum && expectSeqNumRcvd == packet.seqnum) {
        pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);

        // ȡ��Message�����ϵݽ���Ӧ�ò�
        Message msg;
        memcpy(msg.data, packet.payload, sizeof(packet.payload));
        pns->delivertoAppLayer(RECEIVER, msg);

        // ����Ack��, ȷ����ŵ����յ��ı������
        lastAckPkt.acknum   = packet.seqnum;
        lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
        pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);

        expectSeqNumRcvd++;  // �����������1
    }
    else {
        if (checkSum != packet.checksum) {
            pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
        }
        else {
            pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);
        }
        pUtils->printPacket("���շ����·����ϴε�ȷ�ϱ���", lastAckPkt);
    }

    // ����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ���
    pns->sendToNetworkLayer(SENDER, lastAckPkt);
}
