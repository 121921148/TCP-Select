#include "StdAfx.h"
#include "ServerSendSimulate.h"
#include "GPSTcpserver.h"
#include "ServerAppDataHandle.h"


CServerHRSendThread::CServerHRSendThread(void)
{
	InitThread();
}


CServerHRSendThread::~CServerHRSendThread(void)
{
}

DWORD WINAPI CServerHRSendThread::_WorkerThread(LPVOID lpParam)
{
	if (lpParam == NULL)
	{
		TRACE("CServerHRSendThread::_WorkerThread() 获取线程参数失败\n");
		return 0;
	}

	CServerHRSendThread *pDS = (CServerHRSendThread*)lpParam;
	TRACE("%s(%d) start\n", pDS->m_strThreadName, pDS->m_dwThreadID);

	char chHRBuf[1024];  //临时接收缓冲

	while(true)
	{
		if (pDS->m_bThreadExit)
		{
			//线程退出清理工作
			pDS->m_bThreadExit = false;

			TRACE("%s(%d) exit\n", pDS->m_strThreadName, pDS->m_dwThreadID);

			break;  //退出线程函数，终止线程
		}

		//向有效客户端连接中填充心跳报文
		CGPSTcpserver *pGpsServer = pDS->m_pGpsServer;
		pGpsServer->m_stArrLock.lock();
		int nCount = pGpsServer->m_arrClientConnects.GetCount();
		if (nCount != 0)
		{
			for (int i = 0; i < nCount; i++)
			{
				if (pGpsServer->m_arrClientConnects[i]->m_pApp)
				{
					memset(chHRBuf, 0, sizeof(chHRBuf));
					sprintf_s(chHRBuf, "Client-%d, server sending HR\n", pGpsServer->m_arrClientConnects[i]->m_pApp->m_stAppInfo.nAppID);
					pGpsServer->m_arrClientConnects[i]->m_stSendBuf.Push((unsigned char*)chHRBuf, 1024);
				}
			}
		}
		else
		{
			Sleep(2000);
		}
		pGpsServer->m_stArrLock.unlock();

		Sleep(1000);
	}

	return 0;
}

bool CServerHRSendThread::Start(DWORD dwIP, WORD wPort)
{
	//申请线程句柄，启动线程，程序退出时释放
	m_bThreadExit = false;
	m_HWorkerThread = ::CreateThread(0, 0, _WorkerThread, (void *)m_pThreadParam, 0, &m_dwThreadID);
	if (m_HWorkerThread == NULL)
	{
		TRACE("CServerHRSendThread::Start() m_HWorkerThread create failed\n");
		return false;
	}

	return true;
}

bool CServerHRSendThread::Puase()
{
	return true;
}

bool CServerHRSendThread::Exit()
{
	if (m_HWorkerThread != NULL)  //说明线程还在运行
	{
		m_bThreadExit = true;

		//等待线程退出
		DWORD dw = WaitForSingleObject(m_HWorkerThread, 5000);  //等待一个进程结束
		switch (dw)
		{
		case WAIT_OBJECT_0:  // m_HWorkerThread所代表的线程在5秒内结束
			//关闭线程句柄
			if (m_HWorkerThread)
			{
				CloseHandle(m_HWorkerThread);
				m_HWorkerThread = NULL;
			}
			TRACE("线程-%d 在5秒内结束\n", m_dwThreadID);
			break;
		case WAIT_TIMEOUT:  // 等待时间超过5秒
			TRACE("线程结束超时，停止等待\n");
			break;
		case WAIT_FAILED:  // 函数调用失败，比如传递了一个无效的句柄
			TRACE("函数调用失败，可能是线程句柄无效\n");
			break;
		}
	}

	return true;
}

void CServerHRSendThread::NotifyExit()
{
	if (!m_bThreadExit)
	{
		m_bThreadExit = true;  //线程会执行最近一次循环后退出
	}
}

void CServerHRSendThread::InitThread()
{
	m_HWorkerThread = NULL;
	m_pThreadParam = this;
	m_bThreadExit = false;
	m_strThreadName = "CServerHRSendThread";
}

void CServerHRSendThread::InitDataSource()
{
	m_pGpsServer = NULL;
}

void CServerHRSendThread::ExitThread()
{
	//关闭线程句柄
	if (m_HWorkerThread)
	{
		CloseHandle(m_HWorkerThread);
		m_HWorkerThread = NULL;
	}
}

void CServerHRSendThread::ExitDataSource()
{
}
