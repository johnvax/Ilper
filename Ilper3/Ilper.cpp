
// ------------------------------------------------------------------------------
// ILPER 2.50 for Windows
// Copyright (c) 2008-2013  J-F Garnier
// Visual C++ version by Christoph Giesselink 2013
// Visual C++ MFC 2026 Winapp version by Jean-Michel Vansteene 2026
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ------------------------------------------------------------------------------
//
// Ilper.cpp : Defines the class behaviors for the application.
//


#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "Ilper.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CIlperApp

BEGIN_MESSAGE_MAP(CIlperApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CIlperApp::OnAppAbout)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CIlperApp construction

CIlperApp::CIlperApp() noexcept
{
	SetAppID(_T("Ilper.v2.5.0.0"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	*m_szIniFile = 0;						// no ini file given
	m_lpFilePart = (LPTSTR)const_cast<wchar_t*>(L"");
	m_bStart = FALSE;

	m_nArgc = 0;
	m_ppArgv = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CIlperApp object

CIlperApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CIlperApp initialization

BOOL CIlperApp::InitInstance()
{
	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//	of your final executable, you should remove from the following
	//	the specific initialization routines you do not need.

	SetRegistryKey(_T("EmuTools"));

#if defined _UNICODE
	{
		m_ppArgv = const_cast<LPCTSTR*>(CommandLineToArgvW(GetCommandLine(), &m_nArgc));
	}
#else
	{
		m_nArgc = __argc;					// no. of command line arguments
		m_ppArgv = const_cast<LPCTSTR*>(__argv); // command line arguments
	}
#endif

	// argument decoder
	for (INT nArgc = 2; nArgc <= m_nArgc; ++nArgc)
	{
		if (*m_ppArgv[nArgc - 1] == _T('/'))	// is an option
		{
			if (CString(m_ppArgv[nArgc - 1]).CompareNoCase(_T("/start")) == 0)
			{
				m_bStart = TRUE;
				continue;
			}
		}
		else								// is a filename
		{
			if (GetFullPathName(m_ppArgv[nArgc - 1], ARRAYSIZEOF(m_szIniFile), m_szIniFile, &m_lpFilePart) > 0)
			{
				DWORD dwFileAtt = GetFileAttributes(m_szIniFile);
				if (dwFileAtt != INVALID_FILE_ATTRIBUTES && (dwFileAtt & FILE_ATTRIBUTE_DIRECTORY) != 0)
				{
					*m_szIniFile = 0;		// no file or file is directory
				}
			}
		}
	}

	// Create the main Window
	CFrameWnd* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// crée et charge le frame avec ses ressources
	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW, nullptr, nullptr);

	// The single window has been initialized and can therefore be displayed and updated.
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	return TRUE;
}

int CIlperApp::ExitInstance()
{
	//TODO: manage the additional resources you have added
	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// Dialog box data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge de DDX/DDV

// Implémentation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App Command to launch Dialog Box
void CIlperApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}



