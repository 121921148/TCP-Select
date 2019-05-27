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
	
	//更新到界面
	if (m_pMainDlg->GetRecvCheck())
	{
		PUPDATE_RCVDATA_INFO pRcvDataInfo = new UPDATE_RCVDATA_INFO;  //窗口消息中释放内存
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
				PUPDATE_RCVDATA_INFO pRcvDataInfo = new UPDATE_RCVDATA_INFO;  //窗口消息中释放内存
				ASSERT(pRcvDataInfo);
				pRcvDataInfo->strTime = m_pMainDlg->GetCurSysTime();
				pRcvDataInfo->strAddr.Format("%s%d", inet_ntoa(m_pAttachedConnect->m_ClientAddr.sin_addr), ntohs(m_pAttachedConnect->m_ClientAddr.sin_port));
				pRcvDataInfo->strData.Format("上线");
				m_pMainDlg->PostMessage(WM_MAINDLG_UPDATE_REV_DATA, NULL, (LPARAM)pRcvDataInfo);

				TRACE("APP-%d 收到网络保活通知，进行应用层相应网络上线处理。。。。\n");
			}
			*/
			TRACE("APP-%d 收到网络保活通知，进行应用层相应网络上线处理。。。。\n", this->m_stAppInfo.nAppID);
			break;
		case CONNECT_CLOSE:
			/*{
				PUPDATE_RCVDATA_INFO pRcvDataInfo = new UPDATE_RCVDATA_INFO;  //窗口消息中释放内存
				ASSERT(pRcvDataInfo);
				pRcvDataInfo->strTime = m_pMainDlg->GetCurSysTime();
				pRcvDataInfo->strAddr.Format("%s%d", inet_ntoa(m_pAttachedConnect->m_ClientAddr.sin_addr), ntohs(m_pAttachedConnect->m_ClientAddr.sin_port));
				pRcvDataInfo->strData.Format("下线");
				m_pMainDlg->PostMessage(WM_MAINDLG_UPDATE_REV_DATA, NULL, (LPARAM)pRcvDataInfo);

				TRACE("APP-%d 收到网络关闭通知，进行应用层相应网络掉线处理。。。。\n");
			}
			*/
			TRACE("APP-%d 收到网络关闭通知，进行应用层相应网络掉线处理。。。。\n", this->m_stAppInfo.nAppID);
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
