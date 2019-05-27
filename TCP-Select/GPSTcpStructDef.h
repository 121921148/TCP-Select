#ifndef _GPSTCPSTRUCTDEF_
#define _GPSTCPSTRUCTDEF_
#pragma once

#include "StdAfx.h"
#include "GPSDataQueue.h"

class CServerAppDataHandle;
class CGPSTcpDataProcess;

#define WM_MAINDLG_UPDATE_CONNECT WM_USER + 1  //主窗口更新TCP连接信息
#define WM_MAINDLG_UPDATE_REV_DATA WM_USER + 2  //主窗口更新客户端接收数据

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
	unsigned char m_Buf[8192];  //每个连接对应的发送数据缓冲，应用层在需要的地方打包，并将打包数据拷贝至此
	unsigned short m_wLen;  //发送缓冲中数据的长度
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
			TRACE("_MUTEX_BUF.Push(): 缓冲溢出\n");
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
			if (nFetchLen == 0)  //缓冲数据全部取走
			{
				memcpy(pDstBuf, m_Buf, m_wLen);
				wDstLen = m_wLen;
				m_wLen = 0;
			}
			else  //取走指定长度的缓冲数据
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
	SOCKET      m_Socket;  // 每一个客户端连接的Socket
	struct sockaddr_in m_ClientAddr;  // 客户端的地址
	//CRecvBedDataThread *m_pAppBedData;  //应用程序类
	CGPSDataQueue m_clRecvDataQ;  //接收数据队列
	MUTEX_BUF m_stSendBuf;  //发送缓冲
	bool bAlive;  //建立连接&心跳开始发送
	DWORD dwLastAliveTick;  //上次心跳保活时间
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
	CONNECT_CREATE = 1,  //连接建立
	CONNECT_ALIVE = 2,  //连接保活（连接建立&开始正常发送数据）
	CONNECT_CLOSE = 3,  //连接关闭
	CONNECT_ERROR = 4,  //连接错误
}NET_STATUS;

typedef struct UPDATE_CONNECTS_INFO
{
	int nTotalConnects;  //连接总数
	struct sockaddr_in ClientAddr;  //客户端地址
	unsigned char byOperateType;  //操作类型：0-删除所有连接信息，1-增加当前连接信息，2-删除当前连接信息
}UPDATE_CONNECTS_INFO, *PUPDATE_CONNECTS_INFO;

typedef struct UPDATE_RCVDATA_INFO
{
	CString strTime;  //时间
	CString strAddr;  //地址
	CString strData;  //数据
}UPDATE_RCVDATA_INFO, *PUPDATE_RCVDATA_INFO;

#endif  // _GPSTCPSTRUCTDEF_