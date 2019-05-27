#pragma once

class CGPSTcpserver;

class CServerHRSendThread
{
public:
	CServerHRSendThread(void);
	~CServerHRSendThread(void);

	//�ڲ��ӿ�
private:
	void InitThread();
	void ExitThread();
	void InitDataSource();
	void ExitDataSource();

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

	//�������
public:
	CGPSTcpserver *m_pGpsServer;
};

