
// MFCPR.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMFCPRApp: 
// �йش����ʵ�֣������ MFCPR.cpp
//

class CMFCPRApp : public CWinApp
{
public:
	CMFCPRApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMFCPRApp theApp;