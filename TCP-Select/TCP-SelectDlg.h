
// TCP-SelectDlg.h : 头文件
//
#include "GPSTcpserver.h"
#include "ServerAppDataHandle.h"

#pragma once


// CTCPSelectDlg 对话框
class CTCPSelectDlg : public CDialogEx
{
// 构造
public:
	CTCPSelectDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_TCPSELECT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonPause();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedOk();
	LRESULT OnUpdateTcpConnects(WPARAM wParam,LPARAM lParam);  //更新连接信息
	LRESULT OnUpdateTcpRcvdata(WPARAM wParam,LPARAM lParam);  //更新连接信息

public:
	CGPSTcpserver m_GpsTcpserver;
	CServerAppDataHandle m_SvrAppData[GPS_MAX_TCPS + 1]; 
	CIPAddressCtrl m_controlListenIP;
	CComboBox m_ctrlComBoxConnects;
	CListCtrl m_listClientRecvData;
	CString GetCurSysTime();
	CComboBox m_comboxBindAppType;
	unsigned char m_byBindType;  //0-上线顺序，1-应用索引
	afx_msg void OnCbnSelchangeComboBindType();
	CButton m_checkConnect;
	CButton m_checkRecv;
	bool GetConnectCheck();
	bool GetRecvCheck();
};
