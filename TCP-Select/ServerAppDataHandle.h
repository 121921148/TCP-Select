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
	PGPS_TCP_CONNECT m_pAttachedConnect;  //������TCP����
	APP_INFO m_stAppInfo;  //Ӧ�ò����ݴ������Ϣ�����봲�Ź���
	CTCPSelectDlg *m_pMainDlg;
	
public:
	bool AttachTcpConnect(PGPS_TCP_CONNECT pConnect);  //����ĳ��TCP����
	void AppDataHandle(unsigned char *pBuf, int nLen);  //Ӧ�ò����ݴ���ص�
	void AppNetStatusHandle(unsigned char byNetStatus);  //Ӧ�ò�����״̬����ģ�ⴲλ�����ߴ���
};

