#pragma once
#include "Cfifo.h"
#include "RdtReceiver.h"

class SRRdtReceiver : public RdtReceiver
{
public:
    static const int WindowSize = 16;  // ���ڴ�С

private:
    int    baseSeqNum;  // �����
    Packet lastAckPkt;  // �ϴη��͵�ȷ�ϱ���

    struct ReceivablePacket
    {
        Packet packet;
        bool   received;  // ����bool����ָʾ�Ƿ���ܹ�
    };
    CFifo<ReceivablePacket, WindowSize> rbuf;  // �������ݰ�������

public:
    SRRdtReceiver();
    virtual ~SRRdtReceiver();

public:
    // ���ձ��ģ�����NetworkService����
    void receive(const Packet &packet);
};