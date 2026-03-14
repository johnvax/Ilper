//////////////////////////////////////////////////////////////////////
// ILPER 2.50 for Windows
// Copyright (c) 2008-2012  J-F Garnier
// Visual C++ version by Christoph Giesselink 2012
// Visual C++ MFC Winapp version by Jean-Michel Vansteene 2026
//
// ILSetup.cpp : implementation of the CILSetup class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "ilper.h"
#include "ILSetup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILSetup dialog


CILSetup::CILSetup(CWnd* pParent /*=NULL*/)
	: CDialog(CILSetup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CILSetup)
	m_strAddrOut = _T("");
	m_uPortOut	 = 0;
	m_uPortIn	 = 0;
	//}}AFX_DATA_INIT
}

void CILSetup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CILSetup)
	DDX_Text(pDX, IDC_ADDR_OUT, m_strAddrOut);
	DDX_Text(pDX, IDC_PORT_OUT, m_uPortOut);
	DDX_Text(pDX, IDC_PORT_IN, m_uPortIn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CILSetup, CDialog)
	//{{AFX_MSG_MAP(CILSetup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CILSetup message handlers

