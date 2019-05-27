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

	//线程相关
public:
	HANDLE m_HWorkerThread;  // 工作者线程的句柄指针
	static DWORD WINAPI _WorkerThread(LPVOID lpParam);  //线程函数
	void *m_pThreadParam;  //线程参数，可以根据需要定义自己的结构体传入线程，一般是包含对象this的结构
	bool m_bThreadExit;  //线程退出标识
	DWORD m_dwThreadID;  //线程ID
public:
	bool Start(DWORD dwIP = 0, WORD wPort = 0);
	bool Puase();
	bool Exit();  //通知线程退出，并等待直到退出
	void NotifyExit();  //仅通知线程退出
	CString m_strThreadName;

	//数据处理相关
public:
	PGPS_TCP_CONNECT m_pGpsTcpConnect;
	int m_nConnectIndex;
	CGPSTcpserver *m_pGpsTcpServer;  //用于操作和Server相关的数据
	void CloseTcpConnect();
	bool AttachAppDataHandle(int nAppID);  //关联连接和应用数据处理类
	CTCPSelectDlg *m_pMainDlg;
	void AttachMainDlg(CTCPSelectDlg *pMainDlg);  //与主线程关联
	void SetConnectIndex(int nConnectIndex);
	void SetTcpServer(CGPSTcpserver *pGpsTcpServer);
	int m_nAppDataIndex;  //应用数据处理索引
	int GetSimulateMacno(unsigned char *pBuf, int nLen);  //获取模拟机器号

	//内部接口
private:
	void InitThread();
	void ExitThread();
	void InitDataSource();
	void ExitDataSource();
};

