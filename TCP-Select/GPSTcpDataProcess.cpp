#include "StdAfx.h"
#include "GPSTcpDataProcess.h"
#include "ServerAppDataHandle.h"
#include "TCP-Select.h"
#include "TCP-SelectDlg.h"
#include "GPSTcpserver.h"


CGPSTcpDataProcess::CGPSTcpDataProcess(void)
{
	InitThread();
	InitDataSource();
}


CGPSTcpDataProcess::~CGPSTcpDataProcess(void)
{
	ExitThread();
	ExitDataSource();
}

DWORD WINAPI CGPSTcpDataProcess::_WorkerThread(LPVOID lpParam)
{
	if (lpParam == NULL)
	{
		TRACE("CGPSTcpserver::_WorkerThread() 获取线程参数失败\n");
		return 0;
	}

	CGPSTcpDataProcess *pDS = (CGPSTcpDataProcess*)lpParam;
	TRACE("%s(%d) start\n", pDS->m_strThreadName, pDS->m_dwThreadID);

	//网络状态监听初始化
	TIMEVAL timeout = {0,1000000};  //超时1s
	fd_set fdsRead, fdsWrite;
	PGPS_TCP_CONNECT pConnect = pDS->m_pGpsTcpConnect;
	unsigned char byRecvBuf[1024 * 4];  //临时接收缓冲
	unsigned char bySendBuf[1024 * 2];  //临时接收缓冲

	while(true)
	{
		if (pDS->m_bThreadExit)
		{
			//线程退出清理工作
			pDS->m_bThreadExit = false;
			pDS->CloseTcpConnect();

			TRACE("%s(%d) exit\n", pDS->m_strThreadName, pDS->m_dwThreadID);

			break;  //退出线程函数，终止线程
		}

		if (pConnect && pConnect->m_Socket > 0)
		{
			FD_ZERO(&fdsRead);
			FD_ZERO(&fdsWrite);
			FD_SET(pConnect->m_Socket, &fdsRead);
			FD_SET(pConnect->m_Socket, &fdsWrite);

			int nRet = select(0, &fdsRead, &fdsWrite, NULL, &timeout);

			switch(nRet)
			{
			case 0:
				TRACE("Client-%s 超时\n", inet_ntoa(pConnect->m_ClientAddr.sin_addr));
			case SOCKET_ERROR:
				TRACE("Client-%s select()failed,errcode  = %d\n", inet_ntoa(pConnect->m_ClientAddr.sin_addr), GetLastError());
				break;
			default:
				if(FD_ISSET(pConnect->m_Socket, &fdsRead))//可读
				{
					int nRecvLen = recv(pConnect->m_Socket, (char*)byRecvBuf, sizeof(byRecvBuf),0);
					if (nRecvLen > 0)  //正常接收数据
					{
						//将收到的数据放入接收缓冲
						pConnect->m_clRecvDataQ.AddDataBlock(byRecvBuf, nRecvLen);

						//模拟数据处理过程
						byRecvBuf[nRecvLen] = 0;  //保证字符串正常结束，可以打印收到的数据方便测试
						//TRACE("Client-%s recv data：%s\n", inet_ntoa(pConnect->m_ClientAddr.sin_addr), (char*)byRecvBuf);

						if (!pDS->m_pGpsTcpConnect->bAlive)  //说明是第一包数据，绑定TCP连接和应用数据
						{
							int nAppID = -1;
							if (pDS->m_pMainDlg->m_byBindType == 1) 
							{
								//nAppID = byRecvBuf[0] - '0';  //默认数据包首字节为1-66的数字，方便模拟床位调试
								if ((nAppID = pDS->GetSimulateMacno(byRecvBuf, nRecvLen)) < 0)
								{
									TRACE("%s(%d) 获取模拟床位失败\n", pDS->m_strThreadName, pDS->m_dwThreadID);

									pDS->m_bThreadExit = true;
								}
							}
							else if (pDS->m_pMainDlg->m_byBindType == 0) 
							{
								nAppID = pDS->m_nConnectIndex;  //直接使用TCP请求顺序绑定应用数据处理模块
							}
							else
							{
								TRACE("%s(%d) 获取应用数据模块索引失败\n", pDS->m_strThreadName, pDS->m_dwThreadID);
							}

							if (!pDS->AttachAppDataHandle(nAppID))  //绑定失败时，退出线程（清理套接字资源）
							{
								pDS->m_bThreadExit = true;

								break;
							}

							pDS->m_pGpsTcpConnect->m_pApp->AppNetStatusHandle(CONNECT_ALIVE);
							pDS->m_pGpsTcpConnect->bAlive = true;
						}

						;  //应该有组包、解包过程，此处暂时略过
						pDS->m_pGpsTcpConnect->m_pApp->AppDataHandle(byRecvBuf, nRecvLen);  //正常处理数据
						pDS->m_pGpsTcpConnect->dwLastAliveTick = GetTickCount();  //更新保活时间，也可放在APP中心跳处理函数中更新

						//将处理过的数据清除
						pConnect->m_clRecvDataQ.MoveDataBlock(nRecvLen);
					}
					else if (nRecvLen == 0)
					{
						TRACE("Client-%s 关闭套接字\n", inet_ntoa(pConnect->m_ClientAddr.sin_addr));
						pDS->m_bThreadExit = true;  //退出线程
					}
					else if (nRecvLen == -1)
					{
						TRACE("Client-%s recv() failed errcode = %d\n", inet_ntoa(pConnect->m_ClientAddr.sin_addr), GetLastError());
					}
					//OnRecvData();
				}

				if(FD_ISSET(pConnect->m_Socket, &fdsWrite))//可写，向中央机发送数据
				{
					//直接调用发送数据的函数
					if (pConnect->m_stSendBuf.Size() > 0)
					{
						unsigned short wLen = 0;
						pConnect->m_stSendBuf.Fetch(bySendBuf, wLen);
						send(pConnect->m_Socket, (char*)bySendBuf, wLen, 0);
					}
				}				
				break;
			}
            
			;  //连接处理线程可以考虑自己判断保活情况
			Sleep(300);
		}
		else
		{
			TRACE("thread(%d) 运行异常，应该不会出现\n", pDS->m_dwThreadID);
			Sleep(1000);
		}
	}

	return 0;
}

bool CGPSTcpDataProcess::Start(DWORD dwIP, WORD wPort)
{
	//申请线程句柄，启动线程，程序退出时释放
	m_bThreadExit = false;
	m_HWorkerThread = ::CreateThread(0, 0, _WorkerThread, (void *)m_pThreadParam, 0, &m_dwThreadID);
	if (m_HWorkerThread == NULL)
	{
		TRACE("CGPSTcpserver::Start() m_HWorkerThread create failed\n");
		return false;
	}

	return true;
}

bool CGPSTcpDataProcess::Puase()
{
	return true;
}

bool CGPSTcpDataProcess::Exit()
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

void CGPSTcpDataProcess::NotifyExit()
{
	if (!m_bThreadExit)
	{
		m_bThreadExit = true;  //线程会执行最近一次循环后退出
	}
}

void CGPSTcpDataProcess::InitThread()
{
	m_HWorkerThread = NULL;
	m_pThreadParam = this;
	m_bThreadExit = false;
	m_strThreadName.Format("CGPSTcpDataProcess");
}

void CGPSTcpDataProcess::InitDataSource()
{
	m_pGpsTcpConnect = NULL;
	m_pGpsTcpServer = NULL;
	m_nAppDataIndex = -1;
}

void CGPSTcpDataProcess::ExitThread()
{
	//关闭线程句柄
	if (m_HWorkerThread)
	{
		CloseHandle(m_HWorkerThread);
		m_HWorkerThread = NULL;
	}
}

void CGPSTcpDataProcess::ExitDataSource()
{
}

//关闭socket
void CGPSTcpDataProcess::CloseTcpConnect()
{
	if (m_pGpsTcpConnect)
	{
		//应用数据掉线通知
		if (m_pGpsTcpConnect->m_pApp)
		{
			m_pGpsTcpConnect->m_pApp->AppNetStatusHandle(CONNECT_CLOSE);
		}

		//关闭套接字
		shutdown(m_pGpsTcpConnect->m_Socket, SD_BOTH);
		closesocket(m_pGpsTcpConnect->m_Socket);

		//移除连接管理列表，并释放内存
		m_pGpsTcpServer->RemoveClietnConnect(m_pGpsTcpConnect);
		memset(m_pGpsTcpConnect, 0, sizeof(GPS_TCP_CONNECT));
		delete m_pGpsTcpConnect;
		m_pGpsTcpConnect = NULL;
	}
}

bool CGPSTcpDataProcess::AttachAppDataHandle(int nAppID)
{
	if ((nAppID >= 0 && nAppID <= 66) && !m_pMainDlg->m_SvrAppData[nAppID].m_pAttachedConnect)
	{
		m_pMainDlg->m_SvrAppData[nAppID].AttachTcpConnect(m_pGpsTcpConnect);
	}
	else
	{
		TRACE("CGPSTcpDataProcess::AttachAppDataHandle()-nAppID is invalid\n");

		return false;
	}

	return true;
}

void CGPSTcpDataProcess::AttachMainDlg(CTCPSelectDlg *pMainDlg)
{
	if(pMainDlg == NULL)
	{
		TRACE("CGPSTcpserver::AttachMainDlg() failed\n");
	}

	m_pMainDlg = pMainDlg;
}

//绑定该线程处理的连接数索引，方便并发测试
void CGPSTcpDataProcess::SetConnectIndex(int nConnectIndex)
{
	m_nConnectIndex = nConnectIndex;
}

void CGPSTcpDataProcess::SetTcpServer(CGPSTcpserver *pGpsTcpServer)
{
	if (pGpsTcpServer)
	{
		m_pGpsTcpServer = pGpsTcpServer;
	}
}

//获取模拟机器号
int CGPSTcpDataProcess::GetSimulateMacno(unsigned char *pBuf, int nLen)
{
	if (pBuf == NULL || nLen < 0)
	{
		TRACE("CGPSTcpDataProcess::GetSimulateMacno()-parameter is invalid\n");
	}

	char strMacno[3] = {0};
	int k = 0;

	for (int i = 0; i < nLen; i++)
	{
		if ((pBuf[i] > '0' && pBuf[i] <= '9'))  //只取前两个数字字符
		{
			if (i < 2)
			{
				k = i;
				strMacno[i] = pBuf[i];
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	if (k == 0)  //说明首字符为非数字字符
	{
		return -1;
	}
	else
	{
		k = atoi(strMacno);

		if (k >=0 && k < 66) //模拟66个连接
		{
			return k;
		}
		else
		{
			return -1;
		}
	}
}