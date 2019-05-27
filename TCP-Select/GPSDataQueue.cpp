#include "StdAfx.h"
#include "GPSDataQueue.h"
//#include "../IOCP/IOCPModel.h"


CGPSDataQueue::CGPSDataQueue(void)
{
	memset(m_pDataStream, 0, sizeof(m_pDataStream));
	m_nDataLen = 0;
	m_nCurDataIndex = 0;
	//m_pOwner = NULL;
}


CGPSDataQueue::~CGPSDataQueue(void)
{
}

int CGPSDataQueue::Size()
{
	ASSERT(m_nDataLen >= 0 && m_nDataLen <= MAX_BUF_LEN);

	return m_nDataLen;
}

void CGPSDataQueue::AddDataBlock(unsigned char *pchBuf, int nAdBytes)
{
	if (pchBuf != NULL && nAdBytes != 0)
	{
		if (m_nDataLen + nAdBytes < MAX_BUF_LEN)
		{
			memcpy(m_pDataStream + m_nDataLen, pchBuf, nAdBytes);
			m_nDataLen += nAdBytes;
		}
		else
		{
			TRACE("CGPSDataQueue::AddDataBlock(): 队列缓冲溢出!\n");
		}
	}
	else
	{
		TRACE("CGPSDataQueue::AddDataBlock(): 形参验证失败!\n");
		return;
	}
}

void CGPSDataQueue::MoveDataBlock(int nMvBytes)
{
	if (nMvBytes > 0)
	{
		if (m_nDataLen >= nMvBytes)
		{
			int nReserveDataLen = m_nDataLen - nMvBytes;
			if (nReserveDataLen > 0)
			{
				memmove(m_pDataStream, m_pDataStream + nMvBytes, nReserveDataLen);
			}
			else
			{				
				memset(m_pDataStream, 0, sizeof(m_pDataStream));
			}

			m_nDataLen = nReserveDataLen;
			m_nCurDataIndex = 0;
		}
		else
		{
			TRACE("CGPSDataQueue::MoveDataBlock(): 移除数据越界!\n");
		    return;
		}
	}
	else
	{
		TRACE("CGPSDataQueue::MoveDataBlock(): 形参验证失败!\n");
		return;
	}
}

void CGPSDataQueue::Clear()
{
	memset(m_pDataStream, 0, sizeof(m_pDataStream));

	m_nDataLen = 0;
	m_nCurDataIndex = 0;
}

void CGPSDataQueue::Next()
{
	if (m_nCurDataIndex >=0)
	{
		m_nCurDataIndex++;
	}
	else
	{
		TRACE("CGPSDataQueue::MoveDataBlock(): 队列指针无效!\n");
	}
}

//从队列中获取一个完整的报文
bool CGPSDataQueue::GetQElement(unsigned char *pchBuf, int &nMsgLen)
{
	return true;
}

//从队列中获取一个数据
//默认nIndex = -1时，直接获取当前队列指针指向数据，否则获取指定索引数据
unsigned char CGPSDataQueue::Data(int nIndex)
{
	ASSERT(nIndex >=0 && nIndex < m_nDataLen);
		
	m_nCurDataIndex = nIndex;
	return m_pDataStream[nIndex];
}

//队列数据遍历完毕
bool CGPSDataQueue::IsEnd()
{
	return (m_nCurDataIndex >= m_nDataLen - 1) ? true : false;
}

//获取指定位置数据的地址
unsigned char* CGPSDataQueue::GetPData(int i)
{
	if (i >= 0 && i < MAX_BUF_LEN)
	{
		return &m_pDataStream[i];
	}
	else
	{
		return NULL;
	}
}
/*
void CGPSDataQueue::SetOwner(PER_SOCKET_CONTEXT *pOwner)
{
	if (pOwner != NULL)
	{
		m_pOwner = pOwner;
	}
}
*/
/*
PER_SOCKET_CONTEXT* CGPSDataQueue::GetOwner()
{
	return m_pOwner;
}
*/
void CGPSDataQueue::NotifyOwnerBindBed(int nMacNo)
{
	/*
	if (m_pOwner != NULL)
	{
		m_pOwner->BindBed(nMacNo);
	}
	*/
}