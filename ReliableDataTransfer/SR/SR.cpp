#include "Common.h"
#include "SRRdtReceiver.h"
#include "SRRdtSender.h"
#include "Global.h"
#include "RdtReceiver.h"
#include "RdtSender.h"

int main(int argc, char *argv[])
{
    RdtSender *  ps = new SRRdtSender();
    RdtReceiver *pr = new SRRdtReceiver();
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
