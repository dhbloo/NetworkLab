#include "Common.h"
#include "GBNTcpRdtReceiver.h"
#include "GBNTcpRdtSender.h"
#include "Global.h"

int main(int argc, char *argv[])
{
    RdtSender *  ps = new GBNTcpRdtSender();
    RdtReceiver *pr = new GBNTcpRdtReceiver();
    // pns->setRunMode(0);  // VERBOSģʽ
    pns->setRunMode(1);  //����ģʽ
    pns->init();
    pns->setRtdSender(ps);
    pns->setRtdReceiver(pr);
    pns->setInputFile("..\\input.txt");
    pns->setOutputFile("..\\output.txt");

    pns->start();

    delete ps;
    delete pr;
    delete pUtils;  //ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
    delete pns;     //ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete

    return 0;
}
