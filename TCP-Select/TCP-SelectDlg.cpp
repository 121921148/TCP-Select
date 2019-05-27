
// TCP-SelectDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "TCP-Select.h"
#include "TCP-SelectDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTCPSelectDlg �Ի���




CTCPSelectDlg::CTCPSelectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTCPSelectDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTCPSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS_LISTEN, m_controlListenIP);
	DDX_Control(pDX, IDC_COMBO1, m_ctrlComBoxConnects);
	DDX_Control(pDX, IDC_LIST_REV_DATA, m_listClientRecvData);
	DDX_Control(pDX, IDC_COMBO_BIND_TYPE, m_comboxBindAppType);
	DDX_Control(pDX, IDC_CHECK_CONNECT, m_checkConnect);
	DDX_Control(pDX, IDC_CHECK_RECV, m_checkRecv);
}

BEGIN_MESSAGE_MAP(CTCPSelectDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CTCPSelectDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CTCPSelectDlg::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CTCPSelectDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDOK, &CTCPSelectDlg::OnBnClickedOk)
	ON_MESSAGE(WM_MAINDLG_UPDATE_CONNECT,OnUpdateTcpConnects)
	ON_MESSAGE(WM_MAINDLG_UPDATE_REV_DATA,OnUpdateTcpRcvdata)
	ON_CBN_SELCHANGE(IDC_COMBO_BIND_TYPE, &CTCPSelectDlg::OnCbnSelchangeComboBindType)
END_MESSAGE_MAP()


// CTCPSelectDlg ��Ϣ�������

BOOL CTCPSelectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//��ʼ������
	m_GpsTcpserver.AttachMainDlg(this);
	m_GpsTcpserver.InitSocket();
	m_controlListenIP.SetAddress(ntohl(m_GpsTcpserver.GetLocalIP()));
	CString strPort("54000");
	((CEdit*)GetDlgItem(IDC_EDIT_PORT))->SetWindowText(strPort);

	//��ʼ����λ
	for (int i = 0; i < GPS_MAX_TCPS + 1; i++)
	{
		m_SvrAppData[i].m_stAppInfo.nAppID = i;
		m_SvrAppData[i].m_pMainDlg = this;
	}

	//��ʼ��list�ؼ�
	int i;  
	LV_COLUMN lvcolumn;
	TCHAR rgtsz[3][10] = {"ʱ��", "��ַ", "��������"};
	int ncolumnWidth[3] = {150, 150, 400};
	for (i = 0; i < 3; i++)  // add the columns to the list control
	{
		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvcolumn.fmt = LVCFMT_CENTER;
		lvcolumn.pszText = rgtsz[i];
		lvcolumn.iSubItem = i;
		lvcolumn.cx = ncolumnWidth[i];
		this->m_listClientRecvData.InsertColumn(i, &lvcolumn);  // assumes return value is OK.
	}
	DWORD dwStyle = ListView_GetExtendedListViewStyle(m_listClientRecvData);
	//Add the full row select and grid line style to the existing extended styles
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES |
		LVS_EX_HEADERDRAGDROP ;
	ListView_SetExtendedListViewStyle (m_listClientRecvData, dwStyle);

	//combox
	m_comboxBindAppType.AddString("����˳��");
	m_comboxBindAppType.AddString("Ӧ������");
	m_comboxBindAppType.SetCurSel(1);
	m_byBindType = 1;

	//��ѡ��ؼ�
	m_checkConnect.SetCheck(0);
	m_checkRecv.SetCheck(0);

	//������ֹͣ״̬
	((CButton*)GetDlgItem(IDC_BUTTON_START))->EnableWindow(true);
	((CButton*)GetDlgItem(IDC_BUTTON_STOP))->EnableWindow(false);
	((CButton*)GetDlgItem(IDC_BUTTON_PAUSE))->EnableWindow(false);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CTCPSelectDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CTCPSelectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CTCPSelectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTCPSelectDlg::OnBnClickedButtonStart()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DWORD dwIP = 0;
	WORD wPort = 0;
	CString str;

	m_controlListenIP.GetAddress(dwIP);
	((CEdit*)GetDlgItem(IDC_EDIT_PORT))->GetWindowText(str);
	wPort = _ttoi(str);

	m_GpsTcpserver.Start(dwIP, wPort);

	((CButton*)GetDlgItem(IDC_BUTTON_START))->EnableWindow(false);
	((CButton*)GetDlgItem(IDC_BUTTON_STOP))->EnableWindow(true);
	((CButton*)GetDlgItem(IDC_BUTTON_PAUSE))->EnableWindow(false);
}


void CTCPSelectDlg::OnBnClickedButtonPause()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}


void CTCPSelectDlg::OnBnClickedButtonStop()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_GpsTcpserver.Exit();

	((CButton*)GetDlgItem(IDC_BUTTON_STOP))->EnableWindow(false);
	((CButton*)GetDlgItem(IDC_BUTTON_START))->EnableWindow(true);
	((CButton*)GetDlgItem(IDC_BUTTON_PAUSE))->EnableWindow(false);
}


void CTCPSelectDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_GpsTcpserver.Exit();

	CDialogEx::OnOK();
}

LRESULT CTCPSelectDlg::OnUpdateTcpConnects(WPARAM wParam,LPARAM lParam)
{
	PUPDATE_CONNECTS_INFO pConnectInfo = (PUPDATE_CONNECTS_INFO)lParam;
	CString str;
	str.Format("%d", pConnectInfo->nTotalConnects);
	((CEdit*)GetDlgItem(IDC_EDIT_CONNECTS))->SetWindowText(str);

	if (pConnectInfo->byOperateType == 1)  //���������Ϣ
	{
		str.Format("%s:%d", inet_ntoa(pConnectInfo->ClientAddr.sin_addr), ntohs(pConnectInfo->ClientAddr.sin_port));
		m_ctrlComBoxConnects.AddString(str);
	}
	else if (pConnectInfo->byOperateType == 2)  //ɾ��������Ϣ
	{
		str.Format("%s:%d", inet_ntoa(pConnectInfo->ClientAddr.sin_addr), ntohs(pConnectInfo->ClientAddr.sin_port));
		int nitem = m_ctrlComBoxConnects.FindString(-1, str);
		m_ctrlComBoxConnects.DeleteString(nitem);
	}
	else if (pConnectInfo->byOperateType == 0)  //���������Ϣ
	{
		((CEdit*)GetDlgItem(IDC_EDIT_CONNECTS))->SetWindowText("0");
		m_ctrlComBoxConnects.Clear();
	}
	else
	{
		TRACE("CTCPSelectDlg::OnUpdateTcpConnects() not support operating\n");
	}

	if (pConnectInfo)
	{
		delete pConnectInfo;
		pConnectInfo = NULL;
	}

	return 0;
}
LRESULT CTCPSelectDlg::OnUpdateTcpRcvdata(WPARAM wParam,LPARAM lParam)
{
	PUPDATE_RCVDATA_INFO pRcvdataInfo = (PUPDATE_RCVDATA_INFO)lParam;
	CString str;

	int nLines = m_listClientRecvData.GetItemCount();
	if (nLines < 1000)
	{
		m_listClientRecvData.InsertItem(nLines, pRcvdataInfo->strTime);
		m_listClientRecvData.SetItemText(nLines, 1, pRcvdataInfo->strAddr);
		m_listClientRecvData.SetItemText(nLines, 2, pRcvdataInfo->strData);
	}
	else  //���ڵ���1000��ʱ������У��������
	{
		m_listClientRecvData.DeleteAllItems();
		m_listClientRecvData.InsertItem(0, pRcvdataInfo->strTime);
		m_listClientRecvData.SetItemText(0, 1, pRcvdataInfo->strAddr);
		m_listClientRecvData.SetItemText(0, 2, pRcvdataInfo->strData);
	}

	if (pRcvdataInfo)
	{
		delete pRcvdataInfo;
		pRcvdataInfo = NULL;
	}

	return 0;
}

//��ȡϵͳ��ǰʱ��
CString CTCPSelectDlg::GetCurSysTime()
{
	SYSTEMTIME bsTime;
	CString strTime;
	
	::GetLocalTime(&bsTime);
	strTime.Format("%.4d-%.2d-%.2d %.2d:%.2d:%.2d",bsTime.wYear,bsTime.wMonth,bsTime.wDay,bsTime.wHour,bsTime.wMinute,
		                               bsTime.wSecond);
	return	strTime;	
}

void CTCPSelectDlg::OnCbnSelchangeComboBindType()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int nSel = m_comboxBindAppType.GetCurSel();	
	m_byBindType = nSel;
}

bool CTCPSelectDlg::GetConnectCheck()
{
	return m_checkConnect.GetCheck();
}

bool CTCPSelectDlg::GetRecvCheck()
{
	return m_checkRecv.GetCheck();
}