#pragma once
#define MAX_BUF_LEN (16*1024)
//#include "GpsProtocolStrucDef.h"

//typedef struct _PER_SOCKET_CONTEXT PER_SOCKET_CONTEXT;  //GPS-TCP���� socket ����ṹ����

class CGPSDataQueue
{
public:
	CGPSDataQueue(void);
	~CGPSDataQueue(void);

public:
	int m_nDataLen;                 //�������ݳ���
	int m_nCurDataIndex;       //����ָ�루Ҫ���������ڶ����е�������

private:
	unsigned char m_pDataStream[MAX_BUF_LEN];      //����
	//PER_SOCKET_CONTEXT *m_pOwner;

public:
	int Size();   //��ȡ���������ݳ���
	void AddDataBlock(unsigned char *pchBuf, int nAdBytes);    //�������������ݿ�
	void MoveDataBlock(int nRmBytes);    //�ƶ�����������ݿ飨ֱ���Ƶ����ף�
	void Clear();   //��ն�������
	void Next();    //��ȡ����ָ��
	bool GetQElement(unsigned char *pchBuf, int &nMsgLen);  //�Ӷ����л�ȡһ�������ı���
	unsigned char Data(int nIndex = -1);
	bool IsEnd();  //�������ݱ������
	unsigned char* GetPData(int i);  //��ȡָ��λ�����ݵĵ�ַ
	//void SetOwner(PER_SOCKET_CONTEXT *pOwner);
	//PER_SOCKET_CONTEXT* GetOwner();
	void NotifyOwnerBindBed(int nMacNo);
};

