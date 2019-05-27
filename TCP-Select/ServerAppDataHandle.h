#pragma once

#include "GPSTcpStructDef.h"

class CTCPSelectDlg;

typedef struct tagAPP_INFO
{
	int nAppID;
}APP_INFO, *PAPP_INFO;

class CServerAppDataHandle
{
public:
	CServerAppDataHandle(void);
	~CServerAppDataHandle(void);

public:
	PGPS_TCP_CONNECT m_pAttachedConnect;  //关联的TCP连接
	APP_INFO m_stAppInfo;  //应用层数据处理的信息，可与床号关联
	CTCPSelectDlg *m_pMainDlg;
	
public:
	bool AttachTcpConnect(PGPS_TCP_CONNECT pConnect);  //关联某个TCP连接
	void AppDataHandle(unsigned char *pBuf, int nLen);  //应用层数据处理回调
	void AppNetStatusHandle(unsigned char byNetStatus);  //应用层网络状态处理（模拟床位上下线处理）
};

