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
		TRACE("CGPSTcpserver::_WorkerThread() ��ȡ�̲߳���ʧ��\n");
		return 0;
	}

	CGPSTcpDataProcess *pDS = (CGPSTcpDataProcess*)lpParam;
	TRACE("%s(%d) start\n", pDS->m_strThreadName, pDS->m_dwThreadID);

	//����״̬������ʼ��
	TIMEVAL timeout = {0,1000000};  //��ʱ1s
	fd_set fdsRead, fdsWrite;
	PGPS_TCP_CONNECT pConnect = pDS->m_pGpsTcpConnect;
	unsigned char byRecvBuf[1024 * 4];  //��ʱ���ջ���
	unsigned char bySendBuf[1024 * 2];  //��ʱ���ջ���

	while(true)
	{
		if (pDS->m_bThreadExit)
		{
			//�߳��˳�������
			pDS->m_bThreadExit = false;
			pDS->CloseTcpConnect();

			TRACE("%s(%d) exit\n", pDS->m_strThreadName, pDS->m_dwThreadID);

			break;  //�˳��̺߳�������ֹ�߳�
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
				TRACE("Client-%s ��ʱ\n", inet_ntoa(pConnect->m_ClientAddr.sin_addr));
			case SOCKET_ERROR:
				TRACE("Client-%s select()failed,errcode  = %d\n", inet_ntoa(pConnect->m_ClientAddr.sin_addr), GetLastError());
				break;
			default:
				if(FD_ISSET(pConnect->m_Socket, &fdsRead))//�ɶ�
				{
					int nRecvLen = recv(pConnect->m_Socket, (char*)byRecvBuf, sizeof(byRecvBuf),0);
					if (nRecvLen > 0)  //������������
					{
						//���յ������ݷ�����ջ���
						pConnect->m_clRecvDataQ.AddDataBlock(byRecvBuf, nRecvLen);

						//ģ�����ݴ������
						byRecvBuf[nRecvLen] = 0;  //��֤�ַ����������������Դ�ӡ�յ������ݷ������
						//TRACE("Client-%s recv data��%s\n", inet_ntoa(pConnect->m_ClientAddr.sin_addr), (char*)byRecvBuf);

						if (!pDS->m_pGpsTcpConnect->bAlive)  //˵���ǵ�һ�����ݣ���TCP���Ӻ�Ӧ������
						{
							int nAppID = -1;
							if (pDS->m_pMainDlg->m_byBindType == 1) 
							{
								//nAppID = byRecvBuf[0] - '0';  //Ĭ�����ݰ����ֽ�Ϊ1-66�����֣�����ģ�ⴲλ����
								if ((nAppID = pDS->GetSimulateMacno(byRecvBuf, nRecvLen)) < 0)
								{
									TRACE("%s(%d) ��ȡģ�ⴲλʧ��\n", pDS->m_strThreadName, pDS->m_dwThreadID);

									pDS->m_bThreadExit = true;
								}
							}
							else if (pDS->m_pMainDlg->m_byBindType == 0) 
							{
								nAppID = pDS->m_nConnectIndex;  //ֱ��ʹ��TCP����˳���Ӧ�����ݴ���ģ��
							}
							else
							{
								TRACE("%s(%d) ��ȡӦ������ģ������ʧ��\n", pDS->m_strThreadName, pDS->m_dwThreadID);
							}

							if (!pDS->AttachAppDataHandle(nAppID))  //��ʧ��ʱ���˳��̣߳������׽�����Դ��
							{
								pDS->m_bThreadExit = true;

								break;
							}

							pDS->m_pGpsTcpConnect->m_pApp->AppNetStatusHandle(CONNECT_ALIVE);
							pDS->m_pGpsTcpConnect->bAlive = true;
						}

						;  //Ӧ���������������̣��˴���ʱ�Թ�
						pDS->m_pGpsTcpConnect->m_pApp->AppDataHandle(byRecvBuf, nRecvLen);  //������������
						pDS->m_pGpsTcpConnect->dwLastAliveTick = GetTickCount();  //���±���ʱ�䣬Ҳ�ɷ���APP�������������и���

						//����������������
						pConnect->m_clRecvDataQ.MoveDataBlock(nRecvLen);
					}
					else if (nRecvLen == 0)
					{
						TRACE("Client-%s �ر��׽���\n", inet_ntoa(pConnect->m_ClientAddr.sin_addr));
						pDS->m_bThreadExit = true;  //�˳��߳�
					}
					else if (nRecvLen == -1)
					{
						TRACE("Client-%s recv() failed errcode = %d\n", inet_ntoa(pConnect->m_ClientAddr.sin_addr), GetLastError());
					}
					//OnRecvData();
				}

				if(FD_ISSET(pConnect->m_Socket, &fdsWrite))//��д�����������������
				{
					//ֱ�ӵ��÷������ݵĺ���
					if (pConnect->m_stSendBuf.Size() > 0)
					{
						unsigned short wLen = 0;
						pConnect->m_stSendBuf.Fetch(bySendBuf, wLen);
						send(pConnect->m_Socket, (char*)bySendBuf, wLen, 0);
					}
				}				
				break;
			}
            
			;  //���Ӵ����߳̿��Կ����Լ��жϱ������
			Sleep(300);
		}
		else
		{
			TRACE("thread(%d) �����쳣��Ӧ�ò������\n", pDS->m_dwThreadID);
			Sleep(1000);
		}
	}

	return 0;
}

bool CGPSTcpDataProcess::Start(DWORD dwIP, WORD wPort)
{
	//�����߳̾���������̣߳������˳�ʱ�ͷ�
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
	if (m_HWorkerThread != NULL)  //˵���̻߳�������
	{
		m_bThreadExit = true;

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
			TRACE("�߳�-%d ��5���ڽ���\n", m_dwThreadID);
			break;
		case WAIT_TIMEOUT:  // �ȴ�ʱ�䳬��5��
			TRACE("�߳̽�����ʱ��ֹͣ�ȴ�\n");
			break;
		case WAIT_FAILED:  // ��������ʧ�ܣ����紫����һ����Ч�ľ��
			TRACE("��������ʧ�ܣ��������߳̾����Ч\n");
			break;
		}
	}

	return true;
}

void CGPSTcpDataProcess::NotifyExit()
{
	if (!m_bThreadExit)
	{
		m_bThreadExit = true;  //�̻߳�ִ�����һ��ѭ�����˳�
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
	//�ر��߳̾��
	if (m_HWorkerThread)
	{
		CloseHandle(m_HWorkerThread);
		m_HWorkerThread = NULL;
	}
}

void CGPSTcpDataProcess::ExitDataSource()
{
}

//�ر�socket
void CGPSTcpDataProcess::CloseTcpConnect()
{
	if (m_pGpsTcpConnect)
	{
		//Ӧ�����ݵ���֪ͨ
		if (m_pGpsTcpConnect->m_pApp)
		{
			m_pGpsTcpConnect->m_pApp->AppNetStatusHandle(CONNECT_CLOSE);
		}

		//�ر��׽���
		shutdown(m_pGpsTcpConnect->m_Socket, SD_BOTH);
		closesocket(m_pGpsTcpConnect->m_Socket);

		//�Ƴ����ӹ����б����ͷ��ڴ�
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

//�󶨸��̴߳�������������������㲢������
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

//��ȡģ�������
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
		if ((pBuf[i] > '0' && pBuf[i] <= '9'))  //ֻȡǰ���������ַ�
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

	if (k == 0)  //˵�����ַ�Ϊ�������ַ�
	{
		return -1;
	}
	else
	{
		k = atoi(strMacno);

		if (k >=0 && k < 66) //ģ��66������
		{
			return k;
		}
		else
		{
			return -1;
		}
	}
}