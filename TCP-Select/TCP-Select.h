
// TCP-Select.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTCPSelectApp:
// �йش����ʵ�֣������ TCP-Select.cpp
//

class CTCPSelectApp : public CWinApp
{
public:
	CTCPSelectApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTCPSelectApp theApp;