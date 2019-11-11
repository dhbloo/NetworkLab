#pragma once
#include "Cfifo.h"
#include "RdtSender.h"

class GBNRdtSender : public RdtSender
{
public:
    static const int WindowSize = 8;  // ���ڴ�С

private:
    int baseSeqNum;  // �����
    int nextSeqNum;  // ��һ���

    CFifo<Packet, WindowSize> rbuf;  // �������ݰ�������

public:
    // ����Ӧ�ò�������Message����NetworkService����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;
    // �����Ϊ���ͷ����ڵȴ�ȷ��״̬���ʹ����������ܾ�����Message���򷵻�false
    bool send(const Message &message);
    // ����ȷ��Ack������NetworkService����
    void receive(const Packet &ackPkt);
    // Timeout handler������NetworkService����
    void timeoutHandler(int seqNum);
    // ����RdtSender�Ƿ��ڵȴ�״̬��������ͷ����ȴ�ȷ�ϻ��߷��ʹ�������������true
    bool getWaitingState();

public:
    GBNRdtSender();
    virtual ~GBNRdtSender();
};