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
			TRACE("CGPSDataQueue::AddDataBlock(): ���л������!\n");
		}
	}
	else
	{
		TRACE("CGPSDataQueue::AddDataBlock(): �β���֤ʧ��!\n");
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
			TRACE("CGPSDataQueue::MoveDataBlock(): �Ƴ�����Խ��!\n");
		    return;
		}
	}
	else
	{
		TRACE("CGPSDataQueue::MoveDataBlock(): �β���֤ʧ��!\n");
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
		TRACE("CGPSDataQueue::MoveDataBlock(): ����ָ����Ч!\n");
	}
}

//�Ӷ����л�ȡһ�������ı���
bool CGPSDataQueue::GetQElement(unsigned char *pchBuf, int &nMsgLen)
{
	return true;
}

//�Ӷ����л�ȡһ������
//Ĭ��nIndex = -1ʱ��ֱ�ӻ�ȡ��ǰ����ָ��ָ�����ݣ������ȡָ����������
unsigned char CGPSDataQueue::Data(int nIndex)
{
	ASSERT(nIndex >=0 && nIndex < m_nDataLen);
		
	m_nCurDataIndex = nIndex;
	return m_pDataStream[nIndex];
}

//�������ݱ������
bool CGPSDataQueue::IsEnd()
{
	return (m_nCurDataIndex >= m_nDataLen - 1) ? true : false;
}

//��ȡָ��λ�����ݵĵ�ַ
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