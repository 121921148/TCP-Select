#include "StdAfx.h"
#include "GPSTcpserver.h"
#include "ServerAppDataHandle.h"
#include "TCP-Select.h"
#include "TCP-SelectDlg.h"

CGPSTcpserver::CGPSTcpserver(void)
{
	m_HWorkerThread = NULL;
	m_pThreadParam = this;
	m_bThreadExit = false;
	m_strThreadName = "CGPSTcpserver";
	m_sockListen = INVALID_SOCKET;
	m_nConnectIndex = 0;
}


CGPSTcpserver::~CGPSTcpserver(void)
{
	//关闭线程句柄
	if (m_HWorkerThread)
	{
		CloseHandle(m_HWorkerThread);
		m_HWorkerThread = NULL;
	}
}

DWORD WINAPI CGPSTcpserver::_WorkerThread(LPVOID lpParam)
{
	if (lpParam == NULL)
	{
		TRACE("CGPSTcpserver::_WorkerThread() 获取线程参数失败\n");
	}
	CGPSTcpserver *pDS = (CGPSTcpserver*)lpParam;

	TRACE("%s(%d) start\n", pDS->m_strThreadName, pDS->m_dwThreadID);

	//网络状态监听初始化
	struct sockaddr_in	addrAccept;
	SOCKET socketAccept;
	int nAddrLen = sizeof(sockaddr_in);
	fd_set fdsRead;
	TIMEVAL timeout = {1,200};//等待1.2s超时

	while(true)
	{
		if (pDS->m_bThreadExit)
		{
			//线程退出清理工作
			pDS->m_bThreadExit = false;
			closesocket(pDS->m_sockListen);
			pDS->m_sockListen = INVALID_SOCKET;

			TRACE("%s(%d) exit\n", pDS->m_strThreadName, pDS->m_dwThreadID);

			break;  //退出线程函数，终止线程
		}

		FD_ZERO(&fdsRead);
		socketAccept = INVALID_SOCKET;
		FD_SET(pDS->m_sockListen, &fdsRead);
		int nNetStatus=select(pDS->m_sockListen + 1, &fdsRead, NULL, NULL, &timeout);
		switch(nNetStatus)
		{
		case 0:
			TRACE("TCP Server select() timeout 1200ms\n");
			break;
		case SOCKET_ERROR:
			TRACE("TCP Server select() error, errcode = %d\n", GetLastError());
			//CloseAllConnection();
			break;
		default:
			memset(&addrAccept, 0, sizeof(sockaddr_in));
			if((socketAccept = accept(pDS->m_sockListen, (sockaddr*)&addrAccept, &nAddrLen)) != SOCKET_ERROR)
			{
				TRACE("Client %s:%d connected\n", inet_ntoa(addrAccept.sin_addr), ntohs(addrAccept.sin_port));

				//动态申请客户端管理信息
				PGPS_TCP_CONNECT pConnect = new GPS_TCP_CONNECT;
				if (pConnect == NULL)
				{
					closesocket(socketAccept);
					break;
				}

				pConnect->m_Socket = socketAccept;
				pConnect->m_ClientAddr = addrAccept;
				int nDPThreadIndex = -1;
				if (pDS->m_pMainDlg->m_byBindType == 1)  //按协议打包序号对应应用数据处理模块
				{
					pDS->GetUsableDPThead(nDPThreadIndex);
				}
				else if (pDS->m_pMainDlg->m_byBindType == 0)  //按TCP连接请求顺序对应应用数据处理模块
				{
					nDPThreadIndex = pDS->m_nConnectIndex;
				}
				else
				{
					TRACE("线程m_GpsTcpDataProcess的索引获取失败\n");
				}

				if (nDPThreadIndex >= 0 && nDPThreadIndex < 66)
				{
					pConnect->m_pDPThread = &pDS->m_GpsTcpDataProcess[nDPThreadIndex];

					if (pDS->m_pMainDlg->m_byBindType == 0)
					{
						pDS->m_GpsTcpDataProcess[nDPThreadIndex].m_nAppDataIndex = nDPThreadIndex;
					}
				}
				else
				{
					TRACE("Client(%d)-%s:%d 被拒绝， 无可用数据处理线程，可能已经达到最大连接数\n", pDS->m_nConnectIndex, inet_ntoa(addrAccept.sin_addr), ntohs(addrAccept.sin_port));

					//先将当前接受的客户端关掉
					closesocket(pConnect->m_Socket);
					delete pConnect;
					pConnect = NULL;

					break;
				}
				
				pDS->AddToClietnConnectList(pConnect);

				//初始化并启动连接数据处理线程
				pDS->m_GpsTcpDataProcess[nDPThreadIndex].SetTcpServer(pDS);
				pDS->m_GpsTcpDataProcess[nDPThreadIndex].m_pGpsTcpConnect = pConnect;
				pDS->m_GpsTcpDataProcess[nDPThreadIndex].AttachMainDlg(pDS->m_pMainDlg);
				pDS->m_GpsTcpDataProcess[nDPThreadIndex].SetConnectIndex(pDS->m_nConnectIndex);
				pDS->m_GpsTcpDataProcess[nDPThreadIndex].Start();

				if (pDS->m_pMainDlg->m_byBindType == 0)
				{
					pDS->m_nConnectIndex++;  //并发连接测试用，不用时可直接注释
				}
			}
			else
			{
				TRACE("TCP Server, accept() error, code = %d\n", GetLastError());
				continue;
			}
			break;
		}

		//判断所有客户端的保活情况
		//pDS->CheckClientConnectsAlive();

		Sleep(300);
	}

	return 0;
}

bool CGPSTcpserver::Start(DWORD dwIP, WORD wPort)
{
	if (!InitServer(dwIP, wPort))
	{
		TRACE("CGPSTcpserver::Start() 服务器初始化失败\n");
		return false;
	}

	//申请线程句柄，启动线程，程序退出时释放
	m_bThreadExit = false;
	m_HWorkerThread = ::CreateThread(0, 0, _WorkerThread, (void *)m_pThreadParam, 0, &m_dwThreadID);
	if (m_HWorkerThread == NULL)
	{
		TRACE("CGPSTcpserver::Start() m_HWorkerThread create failed\n");
		return false;
	}

	//初始化并启动心跳发送线程
	m_HRSendThread.m_pGpsServer = this;
	m_HRSendThread.Start();

	return true;
}

bool CGPSTcpserver::Puase()
{
	return true;
}

bool CGPSTcpserver::Exit()
{
	if (m_HWorkerThread != NULL)  //说明线程还在运行
	{
		//按线程依赖关系分别退出
		m_HRSendThread.Exit();  //1.先退出HR发送线程
		CloseAllClient();  //2.再关闭所有客户端连接
		m_bThreadExit = true;  //3.最后退出监听线程

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
			TRACE("线程在5秒内结束\n");
			break;
		case WAIT_TIMEOUT:  // 等待时间超过5秒
			TRACE("线程结束超时，停止等待\n");
			break;
		case WAIT_FAILED:  // 函数调用失败，比如传递了一个无效的句柄
			TRACE("函数调用失败，可能是线程句柄无效\n");
			break;
		default:
			break;
		}

		//关闭所有客户端
		CloseAllClient();
	}

	return true;
}

//初始化socket库
bool CGPSTcpserver::InitSocket()
{
	WORD wVersionRequested = MAKEWORD( 2, 2 );
	WSADATA wsaData;
	int err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		/* Tell the user that we could not find a usable	 */
		/* WinSock DLL.										 */
		return FALSE;
	}
	
	/* 
	   Confirm that the WinSock DLL supports 2.2.		
	    Note that if the DLL supports versions greater 
	    than 2.2 in addition to 2.2, it will still return
	    2.2 in wVersion since that is the version we
	    requested.                                        
	*/
	
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
        HIBYTE( wsaData.wVersion ) != 2 ) {
		/* Tell the user that we could not find a usable	 */
		/* WinSock DLL.										 */
		WSACleanup( );
		return	FALSE;; 
	}
	
	return TRUE;
}

// 获得本机的IP地址
DWORD CGPSTcpserver::GetLocalIP()
{
	// 获得本机主机名
	char hostname[MAX_PATH] = {0};
	PHOSTENT pHostInf;
	if(gethostname(hostname,sizeof(hostname))==0)//获得后返回0
	{
		// 取得IP地址列表中的第一个为返回的IP(因为一台主机可能会绑定多个IP)
		if((pHostInf = gethostbyname(hostname)) != NULL)//获得后不为0
		{
			return m_dwLocalIP = inet_addr(inet_ntoa(*(struct in_addr *)pHostInf->h_addr));
		}
	}
	else
	{
		char *pDefAddr = "127.0.0.1";
		return inet_addr(pDefAddr);
	}

	return 0;
}

bool CGPSTcpserver::InitServer(DWORD dwIP, WORD wPort)
{
	if(dwIP == 0 || wPort == 0)
	{
		TRACE("服务器初始化参数无效\n");
		return false;
	}

	//初始化监听地址
	ZeroMemory((char *)&m_addrServer, sizeof(m_addrServer));
	m_addrServer.sin_family = AF_INET;
	//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);  //绑定本地任意可用地址
	m_addrServer.sin_addr.s_addr = htonl(dwIP);    //绑定本地指定地址  
	m_addrServer.sin_port = htons(wPort);
	int nAddrLen = sizeof(sockaddr_in);

	//创建监听套接字并绑定监听地址，结束监听时需要关闭该套接字
	if (m_sockListen == INVALID_SOCKET)
	{
		if((m_sockListen = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR)
		{
			TRACE("CGPSTcpserver::InitServer() create socket failed, errcode = %d\n", GetLastError());
			return false;
		}
		else
		{
			if(bind(m_sockListen, (SOCKADDR*)&m_addrServer, nAddrLen) == SOCKET_ERROR)
			{
				TRACE("CGPSTcpserver::InitServer() bind failed, errcode = %d\n", GetLastError());
				return false;
			}
			else
			{
				if(listen(m_sockListen, SOMAXCONN)==SOCKET_ERROR)
				{
					TRACE("CGPSTcpserver::InitServer() listen() failed errcode = %d\n", GetLastError());
				}

				TRACE("TCP Server Listen successfully: %s:%d\n", inet_ntoa(m_addrServer.sin_addr), ntohs(m_addrServer.sin_port));
			}
		}
	}

	return true;
}

void CGPSTcpserver::CloseAllClient()
{
	for(int i = 0; i < GPS_MAX_TCPS; i++)
	{
		if (!m_bThreadExit && m_GpsTcpDataProcess[i].m_pGpsTcpConnect)
		{
			m_GpsTcpDataProcess[i].Exit();
		}
	}
}

void CGPSTcpserver::AttachMainDlg(CTCPSelectDlg *pMainDlg)
{
	if(pMainDlg == NULL)
	{
		TRACE("CGPSTcpserver::AttachMainDlg() failed\n");
	}

	m_pMainDlg = pMainDlg;
}

// 将客户端的相关信息存储到数组中
void CGPSTcpserver::AddToClietnConnectList(PGPS_TCP_CONNECT pConnect)
{
	m_stArrLock.lock();

	m_arrClientConnects.Add(pConnect);	
	int n = m_arrClientConnects.GetCount();
	
	if (m_pMainDlg->GetConnectCheck())
	{
		PUPDATE_CONNECTS_INFO pConnectInfo = new UPDATE_CONNECTS_INFO;  //主对话框消息里释放
		if (!pConnectInfo)
		{
			TRACE("CGPSTcpserver::AddToClietnConnectList() pConnectInfo new() failed\n");
		}
		pConnectInfo->nTotalConnects = n;
		memcpy(&pConnectInfo->ClientAddr, &pConnect->m_ClientAddr, sizeof(sockaddr_in));
		pConnectInfo->byOperateType = 1;
		m_pMainDlg->PostMessage(WM_MAINDLG_UPDATE_CONNECT, NULL, (LPARAM)pConnectInfo);
	}

	TRACE("客户端%s:%d 建立连接，当前连接%d个\n", inet_ntoa(pConnect->m_ClientAddr.sin_addr), ntohs(pConnect->m_ClientAddr.sin_port), n);

	m_stArrLock.unlock();
}

//	移除某个特定的客户端连接
void CGPSTcpserver::RemoveClietnConnect(PGPS_TCP_CONNECT pConnect)
{
	m_stArrLock.lock();

	int nCount = m_arrClientConnects.GetCount();
	struct sockaddr_in ClientAddr;

	for(int i = 0; i< nCount; i++)
	{
		if( pConnect==m_arrClientConnects.GetAt(i) )
		{
			m_arrClientConnects.RemoveAt(i);

			if (m_pMainDlg->GetConnectCheck())
			{
				PUPDATE_CONNECTS_INFO pConnectInfo = new UPDATE_CONNECTS_INFO;  //主对话框消息里释放
				pConnectInfo->nTotalConnects = nCount - 1;
				memcpy(&pConnectInfo->ClientAddr, &pConnect->m_ClientAddr, sizeof(sockaddr_in));
				pConnectInfo->byOperateType = 2;
				m_pMainDlg->PostMessage(WM_MAINDLG_UPDATE_CONNECT, NULL, (LPARAM)pConnectInfo);
			}

			memcpy(&ClientAddr, &pConnect->m_ClientAddr, sizeof(sockaddr_in));

			break;
		}
	}

	nCount = m_arrClientConnects.GetCount();
	TRACE("客户端%s:%d 关闭连接，当前连接%d个\n", inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port), nCount);

	m_stArrLock.unlock();
}

// 清空客户端信息
void CGPSTcpserver::ClearClietnConnect()
{
	m_stArrLock.lock();

	int nCount = m_arrClientConnects.GetCount();
	for( int i = 0; i < nCount; i++)
	{
		closesocket(m_arrClientConnects[i]->m_Socket);
		delete m_arrClientConnects.GetAt(i);
	}

	m_arrClientConnects.RemoveAll();

	m_stArrLock.unlock();
}

//客户端连接保活检查
void CGPSTcpserver::CheckClientConnectsAlive()
{
	unsigned char arrDisaliveIndex[GPS_MAX_TCPS];
	int k = 0;
	memset(arrDisaliveIndex, 0, sizeof(arrDisaliveIndex));

	m_stArrLock.lock();	

	int nCount = m_arrClientConnects.GetCount();
	for( int i = 0; i < nCount; i++)
	{
		if (m_arrClientConnects[i]->bAlive && GetTickCount() - m_arrClientConnects[i]->dwLastAliveTick > KEEP_LIVE_TIME)
		{
			m_arrClientConnects[i]->m_pApp->AppNetStatusHandle(CONNECT_CLOSE);  //通知应用层数据处理模块进行掉线处理
			m_arrClientConnects[i]->m_pDPThread->NotifyExit();  //通知客户端连接处理线程退出
			arrDisaliveIndex[k++] = i;  //记录超时客户端的索引，后面集中删除
		}
	}

	//集中删除超时客户端连接
	for (int j = 0; j < k; j++)
	{
		if (arrDisaliveIndex[j] != 0)
		{
			RemoveClietnConnect(m_arrClientConnects[arrDisaliveIndex[j]]);
		}
		else
		{
			break;
		}
	}

	m_stArrLock.unlock();
}

CGPSTcpDataProcess* CGPSTcpserver::GetUsableDPThead(int &nInex)
{
	for (int i = 0; i < GPS_MAX_TCPS; i++)
	{
		if (m_GpsTcpDataProcess[i].m_pGpsTcpConnect == NULL)
		{
			nInex = i;
			return &m_GpsTcpDataProcess[i];
		}
	}

	nInex = -1;
	return NULL;
}