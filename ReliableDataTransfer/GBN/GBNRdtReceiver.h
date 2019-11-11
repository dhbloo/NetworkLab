#pragma once
#include "RdtReceiver.h"

class GBNRdtReceiver : public RdtReceiver
{
private:
    int    expectSeqNumRcvd;  // �ڴ��յ�����һ���������
    Packet lastAckPkt;        // �ϴη��͵�ȷ�ϱ���

public:
    GBNRdtReceiver();
    virtual ~GBNRdtReceiver();

public:
    // ���ձ��ģ�����NetworkService����
    void receive(const Packet &packet);
};