//////////////////////////////////////////////////////////////////////
// ILPER 2.50 for Windows
// Copyright (c) 2008-2012  J-F Garnier
// Visual C++ version by Christoph Giesselink 2020
// Visual C++ MFC Winapp version by Jean-Michel Vansteene 2026
//
// LoopCfg.cpp : implementation of the CLoopCfg class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "ilper.h"
#include "LoopCfg.h"
#include "IldevBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoopCfg dialog

CLoopCfg::CLoopCfg(const std::map<CIL *,CString>& mapNames,const std::vector<CIL *>& vecDevices,CWnd* pParent /*=NULL*/)
	: m_mapNames(mapNames)
	, m_vecDevices(vecDevices)
	, CDialog(CLoopCfg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoopCfg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLoopCfg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoopCfg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoopCfg, CDialog)
	//{{AFX_MSG_MAP(CLoopCfg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoopCfg message handlers

BOOL CLoopCfg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	CListBox *pListBox = reinterpret_cast<CListBox *>(GetDlgItem(IDC_DEVICES));

	std::vector<CIL *>::const_iterator itDevice;

	for (itDevice = m_vecDevices.begin(); itDevice != m_vecDevices.end(); itDevice++)
	{
		std::map<CIL *,CString>::const_iterator itName = m_mapNames.find(*itDevice);
		if (itName != m_mapNames.end())
		{
			CString strDeviceInfo;

			CIldevBase *pDevice = dynamic_cast<CIldevBase *>(*itDevice);
			if (pDevice != NULL)			// it's a virtual HP-IL device
			{
				CString strAddr("---");
				WORD wAddr = pDevice->GetAddr();

				if (wAddr != 0)				// assigned device
				{
					if ((wAddr & 0xFF00) == 0)
					{
						// simple address
						strAddr.Format(_T("%u"),wAddr & 0xFF);
					}
					else
					{
						// extended address
						strAddr.Format(_T("%u.%02u"),wAddr & 0xFF,wAddr >> 8);
					}
				}

				strDeviceInfo.Format(_T("%-8s%-16s\t%u\t%s"),
					itName->second,
					pDevice->GetID$(),
					pDevice->GetAID(),
					strAddr);
			}
			else
			{
				strDeviceInfo.Format(_T("%s\t---\t\t---\t---"),
					itName->second);
			}
			pListBox->AddString(strDeviceInfo);
		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
