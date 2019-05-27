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
		TRACE("CServerHRSendThread::_WorkerThread() ��ȡ�̲߳���ʧ��\n");
		return 0;
	}

	CServerHRSendThread *pDS = (CServerHRSendThread*)lpParam;
	TRACE("%s(%d) start\n", pDS->m_strThreadName, pDS->m_dwThreadID);

	char chHRBuf[1024];  //��ʱ���ջ���

	while(true)
	{
		if (pDS->m_bThreadExit)
		{
			//�߳��˳�������
			pDS->m_bThreadExit = false;

			TRACE("%s(%d) exit\n", pDS->m_strThreadName, pDS->m_dwThreadID);

			break;  //�˳��̺߳�������ֹ�߳�
		}

		//����Ч�ͻ��������������������
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
	//�����߳̾���������̣߳������˳�ʱ�ͷ�
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

void CServerHRSendThread::NotifyExit()
{
	if (!m_bThreadExit)
	{
		m_bThreadExit = true;  //�̻߳�ִ�����һ��ѭ�����˳�
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
	//�ر��߳̾��
	if (m_HWorkerThread)
	{
		CloseHandle(m_HWorkerThread);
		m_HWorkerThread = NULL;
	}
}

void CServerHRSendThread::ExitDataSource()
{
}
