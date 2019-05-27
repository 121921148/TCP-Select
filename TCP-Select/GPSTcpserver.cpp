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
	//�ر��߳̾��
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
		TRACE("CGPSTcpserver::_WorkerThread() ��ȡ�̲߳���ʧ��\n");
	}
	CGPSTcpserver *pDS = (CGPSTcpserver*)lpParam;

	TRACE("%s(%d) start\n", pDS->m_strThreadName, pDS->m_dwThreadID);

	//����״̬������ʼ��
	struct sockaddr_in	addrAccept;
	SOCKET socketAccept;
	int nAddrLen = sizeof(sockaddr_in);
	fd_set fdsRead;
	TIMEVAL timeout = {1,200};//�ȴ�1.2s��ʱ

	while(true)
	{
		if (pDS->m_bThreadExit)
		{
			//�߳��˳�������
			pDS->m_bThreadExit = false;
			closesocket(pDS->m_sockListen);
			pDS->m_sockListen = INVALID_SOCKET;

			TRACE("%s(%d) exit\n", pDS->m_strThreadName, pDS->m_dwThreadID);

			break;  //�˳��̺߳�������ֹ�߳�
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

				//��̬����ͻ��˹�����Ϣ
				PGPS_TCP_CONNECT pConnect = new GPS_TCP_CONNECT;
				if (pConnect == NULL)
				{
					closesocket(socketAccept);
					break;
				}

				pConnect->m_Socket = socketAccept;
				pConnect->m_ClientAddr = addrAccept;
				int nDPThreadIndex = -1;
				if (pDS->m_pMainDlg->m_byBindType == 1)  //��Э������Ŷ�ӦӦ�����ݴ���ģ��
				{
					pDS->GetUsableDPThead(nDPThreadIndex);
				}
				else if (pDS->m_pMainDlg->m_byBindType == 0)  //��TCP��������˳���ӦӦ�����ݴ���ģ��
				{
					nDPThreadIndex = pDS->m_nConnectIndex;
				}
				else
				{
					TRACE("�߳�m_GpsTcpDataProcess��������ȡʧ��\n");
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
					TRACE("Client(%d)-%s:%d ���ܾ��� �޿������ݴ����̣߳������Ѿ��ﵽ���������\n", pDS->m_nConnectIndex, inet_ntoa(addrAccept.sin_addr), ntohs(addrAccept.sin_port));

					//�Ƚ���ǰ���ܵĿͻ��˹ص�
					closesocket(pConnect->m_Socket);
					delete pConnect;
					pConnect = NULL;

					break;
				}
				
				pDS->AddToClietnConnectList(pConnect);

				//��ʼ���������������ݴ����߳�
				pDS->m_GpsTcpDataProcess[nDPThreadIndex].SetTcpServer(pDS);
				pDS->m_GpsTcpDataProcess[nDPThreadIndex].m_pGpsTcpConnect = pConnect;
				pDS->m_GpsTcpDataProcess[nDPThreadIndex].AttachMainDlg(pDS->m_pMainDlg);
				pDS->m_GpsTcpDataProcess[nDPThreadIndex].SetConnectIndex(pDS->m_nConnectIndex);
				pDS->m_GpsTcpDataProcess[nDPThreadIndex].Start();

				if (pDS->m_pMainDlg->m_byBindType == 0)
				{
					pDS->m_nConnectIndex++;  //�������Ӳ����ã�����ʱ��ֱ��ע��
				}
			}
			else
			{
				TRACE("TCP Server, accept() error, code = %d\n", GetLastError());
				continue;
			}
			break;
		}

		//�ж����пͻ��˵ı������
		//pDS->CheckClientConnectsAlive();

		Sleep(300);
	}

	return 0;
}

bool CGPSTcpserver::Start(DWORD dwIP, WORD wPort)
{
	if (!InitServer(dwIP, wPort))
	{
		TRACE("CGPSTcpserver::Start() ��������ʼ��ʧ��\n");
		return false;
	}

	//�����߳̾���������̣߳������˳�ʱ�ͷ�
	m_bThreadExit = false;
	m_HWorkerThread = ::CreateThread(0, 0, _WorkerThread, (void *)m_pThreadParam, 0, &m_dwThreadID);
	if (m_HWorkerThread == NULL)
	{
		TRACE("CGPSTcpserver::Start() m_HWorkerThread create failed\n");
		return false;
	}

	//��ʼ�����������������߳�
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
	if (m_HWorkerThread != NULL)  //˵���̻߳�������
	{
		//���߳�������ϵ�ֱ��˳�
		m_HRSendThread.Exit();  //1.���˳�HR�����߳�
		CloseAllClient();  //2.�ٹر����пͻ�������
		m_bThreadExit = true;  //3.����˳������߳�

		//�ȴ��߳��˳�
		DWORD dw = WaitForSingleObject(m_HWorkerThread, 5000);  //�ȴ�һ�����̽���
		switch (dw)
		{
		case WAIT_OBJECT_0:  // m_HWorkerThread��������߳���5���ڽ���
			//�ر��߳̾��
			if (m_HWorkerThread)
			{
				CloseHandle(m_HWorkerThread);
				m_HWorkerThread = NULL;
			}
			TRACE("�߳���5���ڽ���\n");
			break;
		case WAIT_TIMEOUT:  // �ȴ�ʱ�䳬��5��
			TRACE("�߳̽�����ʱ��ֹͣ�ȴ�\n");
			break;
		case WAIT_FAILED:  // ��������ʧ�ܣ����紫����һ����Ч�ľ��
			TRACE("��������ʧ�ܣ��������߳̾����Ч\n");
			break;
		default:
			break;
		}

		//�ر����пͻ���
		CloseAllClient();
	}

	return true;
}

//��ʼ��socket��
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

// ��ñ�����IP��ַ
DWORD CGPSTcpserver::GetLocalIP()
{
	// ��ñ���������
	char hostname[MAX_PATH] = {0};
	PHOSTENT pHostInf;
	if(gethostname(hostname,sizeof(hostname))==0)//��ú󷵻�0
	{
		// ȡ��IP��ַ�б��еĵ�һ��Ϊ���ص�IP(��Ϊһ̨�������ܻ�󶨶��IP)
		if((pHostInf = gethostbyname(hostname)) != NULL)//��ú�Ϊ0
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
		TRACE("��������ʼ��������Ч\n");
		return false;
	}

	//��ʼ��������ַ
	ZeroMemory((char *)&m_addrServer, sizeof(m_addrServer));
	m_addrServer.sin_family = AF_INET;
	//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);  //�󶨱���������õ�ַ
	m_addrServer.sin_addr.s_addr = htonl(dwIP);    //�󶨱���ָ����ַ  
	m_addrServer.sin_port = htons(wPort);
	int nAddrLen = sizeof(sockaddr_in);

	//���������׽��ֲ��󶨼�����ַ����������ʱ��Ҫ�رո��׽���
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

// ���ͻ��˵������Ϣ�洢��������
void CGPSTcpserver::AddToClietnConnectList(PGPS_TCP_CONNECT pConnect)
{
	m_stArrLock.lock();

	m_arrClientConnects.Add(pConnect);	
	int n = m_arrClientConnects.GetCount();
	
	if (m_pMainDlg->GetConnectCheck())
	{
		PUPDATE_CONNECTS_INFO pConnectInfo = new UPDATE_CONNECTS_INFO;  //���Ի�����Ϣ���ͷ�
		if (!pConnectInfo)
		{
			TRACE("CGPSTcpserver::AddToClietnConnectList() pConnectInfo new() failed\n");
		}
		pConnectInfo->nTotalConnects = n;
		memcpy(&pConnectInfo->ClientAddr, &pConnect->m_ClientAddr, sizeof(sockaddr_in));
		pConnectInfo->byOperateType = 1;
		m_pMainDlg->PostMessage(WM_MAINDLG_UPDATE_CONNECT, NULL, (LPARAM)pConnectInfo);
	}

	TRACE("�ͻ���%s:%d �������ӣ���ǰ����%d��\n", inet_ntoa(pConnect->m_ClientAddr.sin_addr), ntohs(pConnect->m_ClientAddr.sin_port), n);

	m_stArrLock.unlock();
}

//	�Ƴ�ĳ���ض��Ŀͻ�������
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
				PUPDATE_CONNECTS_INFO pConnectInfo = new UPDATE_CONNECTS_INFO;  //���Ի�����Ϣ���ͷ�
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
	TRACE("�ͻ���%s:%d �ر����ӣ���ǰ����%d��\n", inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port), nCount);

	m_stArrLock.unlock();
}

// ��տͻ�����Ϣ
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

//�ͻ������ӱ�����
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
			m_arrClientConnects[i]->m_pApp->AppNetStatusHandle(CONNECT_CLOSE);  //֪ͨӦ�ò����ݴ���ģ����е��ߴ���
			m_arrClientConnects[i]->m_pDPThread->NotifyExit();  //֪ͨ�ͻ������Ӵ����߳��˳�
			arrDisaliveIndex[k++] = i;  //��¼��ʱ�ͻ��˵����������漯��ɾ��
		}
	}

	//����ɾ����ʱ�ͻ�������
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