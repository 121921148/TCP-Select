#pragma once

#include "GPSTcpStructDef.h"

class CServerAppDataHandle;
class CTCPSelectDlg;
class CGPSTcpserver;

class CGPSTcpDataProcess
{
public:
	CGPSTcpDataProcess(void);
	~CGPSTcpDataProcess(void);

	//�߳����
public:
	HANDLE m_HWorkerThread;  // �������̵߳ľ��ָ��
	static DWORD WINAPI _WorkerThread(LPVOID lpParam);  //�̺߳���
	void *m_pThreadParam;  //�̲߳��������Ը�����Ҫ�����Լ��Ľṹ�崫���̣߳�һ���ǰ�������this�Ľṹ
	bool m_bThreadExit;  //�߳��˳���ʶ
	DWORD m_dwThreadID;  //�߳�ID
public:
	bool Start(DWORD dwIP = 0, WORD wPort = 0);
	bool Puase();
	bool Exit();  //֪ͨ�߳��˳������ȴ�ֱ���˳�
	void NotifyExit();  //��֪ͨ�߳��˳�
	CString m_strThreadName;

	//���ݴ������
public:
	PGPS_TCP_CONNECT m_pGpsTcpConnect;
	int m_nConnectIndex;
	CGPSTcpserver *m_pGpsTcpServer;  //���ڲ�����Server��ص�����
	void CloseTcpConnect();
	bool AttachAppDataHandle(int nAppID);  //�������Ӻ�Ӧ�����ݴ�����
	CTCPSelectDlg *m_pMainDlg;
	void AttachMainDlg(CTCPSelectDlg *pMainDlg);  //�����̹߳���
	void SetConnectIndex(int nConnectIndex);
	void SetTcpServer(CGPSTcpserver *pGpsTcpServer);
	int m_nAppDataIndex;  //Ӧ�����ݴ�������
	int GetSimulateMacno(unsigned char *pBuf, int nLen);  //��ȡģ�������

	//�ڲ��ӿ�
private:
	void InitThread();
	void ExitThread();
	void InitDataSource();
	void ExitDataSource();
};

