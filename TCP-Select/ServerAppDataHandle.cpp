#include "StdAfx.h"
#include "ServerAppDataHandle.h"
#include "TCP-Select.h"
#include "TCP-SelectDlg.h"


CServerAppDataHandle::CServerAppDataHandle(void)
{
	m_pAttachedConnect = NULL;
}

CServerAppDataHandle::~CServerAppDataHandle(void)
{
}

bool CServerAppDataHandle::AttachTcpConnect(PGPS_TCP_CONNECT pConnect)
{
	if (pConnect == NULL)
	{
		TRACE("CServerAppDataHandle::AttachTcpConnect()-pConnect is invalid\n");
	}

	m_pAttachedConnect = pConnect;
	m_pAttachedConnect->m_pApp = this;

	return true;
}

void CServerAppDataHandle::AppDataHandle(unsigned char *pBuf, int nLen)
{
	if (pBuf == NULL || nLen <= 0)
	{
		TRACE("CServerAppDataHandle::AppDataHandle()-Param is invalid\n");
	}
	
	//���µ�����
	if (m_pMainDlg->GetRecvCheck())
	{
		PUPDATE_RCVDATA_INFO pRcvDataInfo = new UPDATE_RCVDATA_INFO;  //������Ϣ���ͷ��ڴ�
		ASSERT(pRcvDataInfo);
		pRcvDataInfo->strTime = m_pMainDlg->GetCurSysTime();
		pRcvDataInfo->strAddr.Format("%s%d", inet_ntoa(m_pAttachedConnect->m_ClientAddr.sin_addr), ntohs(m_pAttachedConnect->m_ClientAddr.sin_port));
		pRcvDataInfo->strData.Format("%s", (char*)pBuf);
		m_pMainDlg->PostMessage(WM_MAINDLG_UPDATE_REV_DATA, NULL, (LPARAM)pRcvDataInfo);
	}

	TRACE("Bed%d handle tcp connect data: %s\n", m_stAppInfo.nAppID, (char*)pBuf);
}

void CServerAppDataHandle:: AppNetStatusHandle(unsigned char byNetStatus)
{
	if (byNetStatus >= CONNECT_CREATE &&  byNetStatus >= CONNECT_CREATE <= CONNECT_ERROR)
	{
		switch (byNetStatus)
		{
		case CONNECT_CREATE:
			break;
		case CONNECT_ALIVE:
			/*{
				PUPDATE_RCVDATA_INFO pRcvDataInfo = new UPDATE_RCVDATA_INFO;  //������Ϣ���ͷ��ڴ�
				ASSERT(pRcvDataInfo);
				pRcvDataInfo->strTime = m_pMainDlg->GetCurSysTime();
				pRcvDataInfo->strAddr.Format("%s%d", inet_ntoa(m_pAttachedConnect->m_ClientAddr.sin_addr), ntohs(m_pAttachedConnect->m_ClientAddr.sin_port));
				pRcvDataInfo->strData.Format("����");
				m_pMainDlg->PostMessage(WM_MAINDLG_UPDATE_REV_DATA, NULL, (LPARAM)pRcvDataInfo);

				TRACE("APP-%d �յ����籣��֪ͨ������Ӧ�ò���Ӧ�������ߴ���������\n");
			}
			*/
			TRACE("APP-%d �յ����籣��֪ͨ������Ӧ�ò���Ӧ�������ߴ���������\n", this->m_stAppInfo.nAppID);
			break;
		case CONNECT_CLOSE:
			/*{
				PUPDATE_RCVDATA_INFO pRcvDataInfo = new UPDATE_RCVDATA_INFO;  //������Ϣ���ͷ��ڴ�
				ASSERT(pRcvDataInfo);
				pRcvDataInfo->strTime = m_pMainDlg->GetCurSysTime();
				pRcvDataInfo->strAddr.Format("%s%d", inet_ntoa(m_pAttachedConnect->m_ClientAddr.sin_addr), ntohs(m_pAttachedConnect->m_ClientAddr.sin_port));
				pRcvDataInfo->strData.Format("����");
				m_pMainDlg->PostMessage(WM_MAINDLG_UPDATE_REV_DATA, NULL, (LPARAM)pRcvDataInfo);

				TRACE("APP-%d �յ�����ر�֪ͨ������Ӧ�ò���Ӧ������ߴ���������\n");
			}
			*/
			TRACE("APP-%d �յ�����ر�֪ͨ������Ӧ�ò���Ӧ������ߴ���������\n", this->m_stAppInfo.nAppID);
			m_pAttachedConnect = NULL;
			break;
		case CONNECT_ERROR:
			break;
		default:
			break;
		}
	}
	else
	{
		TRACE("CServerAppDataHandle::AppNetStatusHandle()-Param is invalid\n");
		return;
	}
}
