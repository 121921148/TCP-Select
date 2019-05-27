#pragma once

// winsock 2 ��ͷ�ļ��Ϳ�
#include <winsock2.h>
#include "GPSDataQueue.h"
#include "GPSTcpDataProcess.h"
#include "ServerSendSimulate.h"

//#define MANAGE_THREAD_CONNECT  //ͨ�������߳����ͻ�������

class CTCPSelectDlg;
class CServerHRSendThread;

#define DEFAULT_IP            _T("127.0.0.1")
#define GPS_MAX_TCPS    66  //���֧��66��TCP����
#define KEEP_LIVE_TIME    1000 * 10  //����ʱ��10s

class CGPSTcpserver
{
public:
	CGPSTcpserver(void);
	~CGPSTcpserver(void);

	//�߳����
public:
	HANDLE m_HWorkerThread;  // �������̵߳ľ��ָ��
	static DWORD WINAPI _WorkerThread(LPVOID lpParam);  //�̺߳���
	void *m_pThreadParam;  //�̲߳��������Ը�����Ҫ�����Լ��Ľṹ�崫���̣߳�һ���ǰ�������this�Ľṹ
	bool m_bThreadExit;  //�߳��˳���ʶ
	DWORD m_dwThreadID;  //�߳�ID
public:
	bool Start(DWORD dwIP, WORD wPort);
	bool Puase();
	bool Exit();
	CString m_strThreadName;


	//����������
public:
	SOCKET  m_sockListen;
	struct sockaddr_in m_addrServer;
	WORD m_dwPort;
	DWORD m_dwLocalIP;
public:
	DWORD GetLocalIP();
	bool InitSocket();
	bool InitServer(DWORD dwIP, WORD wPort);

	//�ͻ������ݴ���
public:
	CGPSTcpDataProcess m_GpsTcpDataProcess[GPS_MAX_TCPS];
#ifdef MANAGE_THREAD_CONNECT
	CArray<CGPSTcpDataProcess*>  m_arrClientConnects;  // ��Ч�ͻ������ӹ�������
#else
	CArray<PGPS_TCP_CONNECT>  m_arrClientConnects;  // ��Ч�ͻ������ӹ�������
#endif
	CS_LOCK m_stArrLock;  //m_arrClientConnects���ʿ���
	void AddToClietnConnectList(PGPS_TCP_CONNECT pConnect);
	void RemoveClietnConnect(PGPS_TCP_CONNECT pConnect);
	void ClearClietnConnect();
	void CloseAllClient();
	void CheckClientConnectsAlive();
	CGPSTcpDataProcess* GetUsableDPThead(int &nInex);
	CServerHRSendThread m_HRSendThread;
	int m_nConnectIndex;  //��¼ÿ����������˳�򣬷��㲢������

	//�����̹߳���
public:
	CTCPSelectDlg *m_pMainDlg;
	void AttachMainDlg(CTCPSelectDlg *pMainDlg);
};

