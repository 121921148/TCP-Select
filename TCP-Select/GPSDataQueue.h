#pragma once
#define MAX_BUF_LEN (16*1024)
//#include "GpsProtocolStrucDef.h"

//typedef struct _PER_SOCKET_CONTEXT PER_SOCKET_CONTEXT;  //GPS-TCP连接 socket 管理结构声明

class CGPSDataQueue
{
public:
	CGPSDataQueue(void);
	~CGPSDataQueue(void);

public:
	int m_nDataLen;                 //缓冲数据长度
	int m_nCurDataIndex;       //队列指针（要操作数据在队列中的索引）

private:
	unsigned char m_pDataStream[MAX_BUF_LEN];      //缓冲
	//PER_SOCKET_CONTEXT *m_pOwner;

public:
	int Size();   //获取缓冲中数据长度
	void AddDataBlock(unsigned char *pchBuf, int nAdBytes);    //向队列中添加数据块
	void MoveDataBlock(int nRmBytes);    //移动操作完的数据块（直接移到队首）
	void Clear();   //清空队列数据
	void Next();    //获取队列指针
	bool GetQElement(unsigned char *pchBuf, int &nMsgLen);  //从队列中获取一个完整的报文
	unsigned char Data(int nIndex = -1);
	bool IsEnd();  //队列数据遍历完毕
	unsigned char* GetPData(int i);  //获取指定位置数据的地址
	//void SetOwner(PER_SOCKET_CONTEXT *pOwner);
	//PER_SOCKET_CONTEXT* GetOwner();
	void NotifyOwnerBindBed(int nMacNo);
};

