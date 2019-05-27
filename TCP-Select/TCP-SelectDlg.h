
// TCP-SelectDlg.h : ͷ�ļ�
//
#include "GPSTcpserver.h"
#include "ServerAppDataHandle.h"

#pragma once


// CTCPSelectDlg �Ի���
class CTCPSelectDlg : public CDialogEx
{
// ����
public:
	CTCPSelectDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_TCPSELECT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
	LRESULT OnUpdateTcpConnects(WPARAM wParam,LPARAM lParam);  //����������Ϣ
	LRESULT OnUpdateTcpRcvdata(WPARAM wParam,LPARAM lParam);  //����������Ϣ

public:
	CGPSTcpserver m_GpsTcpserver;
	CServerAppDataHandle m_SvrAppData[GPS_MAX_TCPS + 1]; 
	CIPAddressCtrl m_controlListenIP;
	CComboBox m_ctrlComBoxConnects;
	CListCtrl m_listClientRecvData;
	CString GetCurSysTime();
	CComboBox m_comboxBindAppType;
	unsigned char m_byBindType;  //0-����˳��1-Ӧ������
	afx_msg void OnCbnSelchangeComboBindType();
	CButton m_checkConnect;
	CButton m_checkRecv;
	bool GetConnectCheck();
	bool GetRecvCheck();
};
