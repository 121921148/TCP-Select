#pragma once

// winsock 2 的头文件和库
#include <winsock2.h>
#include "GPSDataQueue.h"
#include "GPSTcpDataProcess.h"
#include "ServerSendSimulate.h"

//#define MANAGE_THREAD_CONNECT  //通过管理线程来客户端连接

class CTCPSelectDlg;
class CServerHRSendThread;

#define DEFAULT_IP            _T("127.0.0.1")
#define GPS_MAX_TCPS    66  //最多支持66条TCP连接
#define KEEP_LIVE_TIME    1000 * 10  //保活时间10s

class CGPSTcpserver
{
public:
	CGPSTcpserver(void);
	~CGPSTcpserver(void);

	//线程相关
public:
	HANDLE m_HWorkerThread;  // 工作者线程的句柄指针
	static DWORD WINAPI _WorkerThread(LPVOID lpParam);  //线程函数
	void *m_pThreadParam;  //线程参数，可以根据需要定义自己的结构体传入线程，一般是包含对象this的结构
	bool m_bThreadExit;  //线程退出标识
	DWORD m_dwThreadID;  //线程ID
public:
	bool Start(DWORD dwIP, WORD wPort);
	bool Puase();
	bool Exit();
	CString m_strThreadName;


	//网络服务监听
public:
	SOCKET  m_sockListen;
	struct sockaddr_in m_addrServer;
	WORD m_dwPort;
	DWORD m_dwLocalIP;
public:
	DWORD GetLocalIP();
	bool InitSocket();
	bool InitServer(DWORD dwIP, WORD wPort);

	//客户端数据处理
public:
	CGPSTcpDataProcess m_GpsTcpDataProcess[GPS_MAX_TCPS];
#ifdef MANAGE_THREAD_CONNECT
	CArray<CGPSTcpDataProcess*>  m_arrClientConnects;  // 有效客户端连接管理数组
#else
	CArray<PGPS_TCP_CONNECT>  m_arrClientConnects;  // 有效客户端连接管理数组
#endif
	CS_LOCK m_stArrLock;  //m_arrClientConnects访问控制
	void AddToClietnConnectList(PGPS_TCP_CONNECT pConnect);
	void RemoveClietnConnect(PGPS_TCP_CONNECT pConnect);
	void ClearClietnConnect();
	void CloseAllClient();
	void CheckClientConnectsAlive();
	CGPSTcpDataProcess* GetUsableDPThead(int &nInex);
	CServerHRSendThread m_HRSendThread;
	int m_nConnectIndex;  //记录每个连接来的顺序，方便并发测试

	//与主线程关联
public:
	CTCPSelectDlg *m_pMainDlg;
	void AttachMainDlg(CTCPSelectDlg *pMainDlg);
};

