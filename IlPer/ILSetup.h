#if !defined(AFX_ILSETUP_H__C93EF6AE_7A5D_49AF_8769_6517F4E7D9EF__INCLUDED_)
#define AFX_ILSETUP_H__C93EF6AE_7A5D_49AF_8769_6517F4E7D9EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILSetup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILSetup dialog

class CILSetup : public CDialog
{
// Construction
public:
	CILSetup(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CILSetup)
	enum { IDD = IDD_TCPIP_SETTING };
	CString m_strAddrOut;
	UINT	m_uPortOut;
	UINT	m_uPortIn;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILSetup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CILSetup)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ILSETUP_H__C93EF6AE_7A5D_49AF_8769_6517F4E7D9EF__INCLUDED_)
