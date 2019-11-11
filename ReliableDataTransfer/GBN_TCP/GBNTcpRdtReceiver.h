#pragma once
#include "RdtReceiver.h"

class GBNTcpRdtReceiver : public RdtReceiver
{
private:
    int    expectSeqNumRcvd;  // �ڴ��յ�����һ���������
    Packet lastAckPkt;        // �ϴη��͵�ȷ�ϱ���

public:
    GBNTcpRdtReceiver();
    virtual ~GBNTcpRdtReceiver();

public:
    // ���ձ��ģ�����NetworkService����
    void receive(const Packet &packet);
};