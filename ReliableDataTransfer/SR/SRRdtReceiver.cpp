#include "SRRdtReceiver.h"

#include "Common.h"
#include "Global.h"

#include <algorithm>

SRRdtReceiver::SRRdtReceiver() : baseSeqNum(0)
{
    // ��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
    lastAckPkt.acknum   = -1;
    lastAckPkt.seqnum   = -1;  // ���Ը��ֶ�
    lastAckPkt.checksum = 0;
    std::fill_n(lastAckPkt.payload, Configuration::PAYLOAD_SIZE, '.');
    lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);

    while (!rbuf.full())  // ��ʼ�����ջ��λ�����
        rbuf.put({{}, false});
}

SRRdtReceiver::~SRRdtReceiver() {}

void SRRdtReceiver::receive(const Packet &packet)
{
    // ����У���
    int checkSum = pUtils->calculateCheckSum(packet);

    // ���������ڽ�������ƫ����
    unsigned long offset = packet.seqnum - baseSeqNum;

    // ���У�����ȷ��ͬʱ�յ����ĵ�����ڿɽ��շ�Χ��
    if (checkSum == packet.checksum && offset < WindowSize) {
        if (!rbuf[offset].received) {  // ����ð���û���չ�
            rbuf[offset].packet   = packet;
            rbuf[offset].received = true;  // �����ݰ����Ϊ�ѽ���
        }

        // ����Ack��, ȷ����ŵ����յ��ı������
        lastAckPkt.acknum   = packet.seqnum;
        lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
        pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
        // ����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ���
        pns->sendToNetworkLayer(SENDER, lastAckPkt);

        std::stringstream msg;
        msg << "���շ���ȷ�յ����ͷ��ı���, �ƶ���������(" << baseSeqNum << "," << baseSeqNum + rbuf.capacity();

        if (offset == 0) {                                   // ����յ��İ����Ϊbase
            while (!rbuf.empty() && rbuf.head().received) {  // ������ſ�ʼ����ȷ�ϱ��ݽ�
                // ȡ��Message�����ϵݽ���Ӧ�ò�
                Message msg;
                memcpy(msg.data, rbuf.get().packet.payload, sizeof(packet.payload));
                pns->delivertoAppLayer(RECEIVER, msg);

                baseSeqNum++;           // �ѽ�����ż�1
                rbuf.put({{}, false});  // ����������һ����λ
            }
        }

        msg << " -> " << baseSeqNum << "," << baseSeqNum + rbuf.capacity() << ")\n";
        pUtils->printPacket(msg.str().c_str(), packet);
    }
    else {
        if (checkSum != packet.checksum) {
            pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
        }
        else {
            if (packet.seqnum - baseSeqNum <= WindowSize) {  // �յ���ȷ�Ϸ�Χ�ڵİ�
                // ����Ack��, ȷ����ŵ����յ��ı������
                lastAckPkt.acknum   = packet.seqnum;
                lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
                pUtils->printPacket("���շ��յ��ѽ��յı���,�ط�ACK", lastAckPkt);
                pns->sendToNetworkLayer(SENDER, lastAckPkt);
            }
            else
                pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);
        }
        pUtils->printPacket("���շ����·����ϴε�ȷ�ϱ���", lastAckPkt);
    }
}
