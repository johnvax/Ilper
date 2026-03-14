
// Ilper.h : main header file for the ILPER application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "incluez 'pch.h' avant d'inclure ce fichier pour PCH"
#endif

#include "resource.h"       // main symbols


/////////////////////////////////////////////////////////////////////////////
// CIlperApp:
// See Ilper.cpp for the implementation of this class
//

class CIlperApp : public CWinApp
{
public:
	CIlperApp() noexcept;

	TCHAR	m_szIniFile[_MAX_PATH]; 		// full pathname when ini-file is used
	LPTSTR	m_lpFilePart;					// ini-file name

	BOOL	m_bStart;						// press the "Start" button

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIlperApp)
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implémentation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

private:
	INT 	m_nArgc;						// no. of command line arguments
	LPCTSTR* m_ppArgv;						// command line arguments
};

extern CIlperApp theApp;

