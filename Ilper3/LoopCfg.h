#if !defined(AFX_LOOPCFG_H__288B0D8C_5F4D_4975_B4DC_CD3C082ED517__INCLUDED_)
#define AFX_LOOPCFG_H__288B0D8C_5F4D_4975_B4DC_CD3C082ED517__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoopCfg.h : header file
//

class CIL;

/////////////////////////////////////////////////////////////////////////////
// CLoopCfg dialog

class CLoopCfg : public CDialog
{
// Construction
public:
	CLoopCfg(const std::map<CIL *,CString>& mapNames,
			 const std::vector<CIL *>& vecDevices,
			 CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLoopCfg)
	enum { IDD = IDD_LOOPCONFIG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoopCfg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLoopCfg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	const std::map<CIL *,CString>& m_mapNames;
	const std::vector<CIL *>& m_vecDevices;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOOPCFG_H__288B0D8C_5F4D_4975_B4DC_CD3C082ED517__INCLUDED_)
