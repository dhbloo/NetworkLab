#pragma once
#include "Cfifo.h"
#include "RdtSender.h"

class GBNTcpRdtSender : public RdtSender
{
public:
    static const int WindowSize      = 8;  // ���ڴ�С
    static const int DuplicateCntMax = 3;  // ���������ش�������ACK��

private:
    int baseSeqNum;    // �����
    int nextSeqNum;    // ��һ���
    int duplicateCnt;  // ����ACK����

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
    GBNTcpRdtSender();
    virtual ~GBNTcpRdtSender();
};