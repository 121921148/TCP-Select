#ifndef _GPSTCPSTRUCTDEF_
#define _GPSTCPSTRUCTDEF_
#pragma once

#include "StdAfx.h"
#include "GPSDataQueue.h"

class CServerAppDataHandle;
class CGPSTcpDataProcess;

#define WM_MAINDLG_UPDATE_CONNECT WM_USER + 1  //�����ڸ���TCP������Ϣ
#define WM_MAINDLG_UPDATE_REV_DATA WM_USER + 2  //�����ڸ��¿ͻ��˽�������

typedef struct _CS_LOCK
{
	_CS_LOCK(){InitializeCriticalSection(&m_cs);}
	~_CS_LOCK(){DeleteCriticalSection(&m_cs);}
	void lock(){EnterCriticalSection(&m_cs);}
	void unlock(){LeaveCriticalSection(&m_cs);}

	CRITICAL_SECTION m_cs;
}CS_LOCK, *PCS_LOCK;

typedef struct _MUTEX_BUF
{
	unsigned char m_Buf[8192];  //ÿ�����Ӷ�Ӧ�ķ������ݻ��壬Ӧ�ò�����Ҫ�ĵط����������������ݿ�������
	unsigned short m_wLen;  //���ͻ��������ݵĳ���
	CS_LOCK m_CSLock;

	_MUTEX_BUF()
	{
		m_wLen = 0;
	}

	void Push(unsigned char *pInBuf, int nLen)
	{
		m_CSLock.lock();

		if (m_wLen + nLen < 8192)
		{
			memcpy(m_Buf + m_wLen, pInBuf, nLen);
			m_wLen = m_wLen + nLen;
		}
		else
		{
			TRACE("_MUTEX_BUF.Push(): �������\n");
		}

		m_CSLock.unlock();
	}

	void Fetch(unsigned char *pDstBuf, unsigned short  &wDstLen, int nFetchLen = 0)
	{
		m_CSLock.lock();

		if (m_wLen == 0)
		{
			wDstLen = 0;
		}
		else
		{
			if (nFetchLen == 0)  //��������ȫ��ȡ��
			{
				memcpy(pDstBuf, m_Buf, m_wLen);
				wDstLen = m_wLen;
				m_wLen = 0;
			}
			else  //ȡ��ָ�����ȵĻ�������
			{
				if (m_wLen > nFetchLen)
				{
					memcpy(pDstBuf, m_Buf, nFetchLen);

					m_wLen = m_wLen - nFetchLen;
				}
				else
				{
					memcpy(pDstBuf, m_Buf, m_wLen);

					m_wLen = 0;
				}
			}
		}

		m_CSLock.unlock();
	}

	int Size()
	{
		return m_wLen;
	}
}MUTEX_BUF, *PMUTEX_BUF;


typedef struct _GPS_TCP_CONNECT
{  
	SOCKET      m_Socket;  // ÿһ���ͻ������ӵ�Socket
	struct sockaddr_in m_ClientAddr;  // �ͻ��˵ĵ�ַ
	//CRecvBedDataThread *m_pAppBedData;  //Ӧ�ó�����
	CGPSDataQueue m_clRecvDataQ;  //�������ݶ���
	MUTEX_BUF m_stSendBuf;  //���ͻ���
	bool bAlive;  //��������&������ʼ����
	DWORD dwLastAliveTick;  //�ϴ���������ʱ��
	CServerAppDataHandle *m_pApp;
	CGPSTcpDataProcess *m_pDPThread;

	_GPS_TCP_CONNECT()
	{
		m_Socket = INVALID_SOCKET;
		bAlive = false;
		m_pApp = NULL;
		m_pDPThread = NULL;
	}
}GPS_TCP_CONNECT, *PGPS_TCP_CONNECT;

typedef enum tagsNET_STATUS{
	CONNECT_CREATE = 1,  //���ӽ���
	CONNECT_ALIVE = 2,  //���ӱ�����ӽ���&��ʼ�����������ݣ�
	CONNECT_CLOSE = 3,  //���ӹر�
	CONNECT_ERROR = 4,  //���Ӵ���
}NET_STATUS;

typedef struct UPDATE_CONNECTS_INFO
{
	int nTotalConnects;  //��������
	struct sockaddr_in ClientAddr;  //�ͻ��˵�ַ
	unsigned char byOperateType;  //�������ͣ�0-ɾ������������Ϣ��1-���ӵ�ǰ������Ϣ��2-ɾ����ǰ������Ϣ
}UPDATE_CONNECTS_INFO, *PUPDATE_CONNECTS_INFO;

typedef struct UPDATE_RCVDATA_INFO
{
	CString strTime;  //ʱ��
	CString strAddr;  //��ַ
	CString strData;  //����
}UPDATE_RCVDATA_INFO, *PUPDATE_RCVDATA_INFO;

#endif  // _GPSTCPSTRUCTDEF_