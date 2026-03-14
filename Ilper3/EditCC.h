#if !defined(AFX_EDITCC_H__F25061C6_D99D_49D6_B7FD_253AD1FA29FA__INCLUDED_)
#define AFX_EDITCC_H__F25061C6_D99D_49D6_B7FD_253AD1FA29FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditCC.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditCC window

class CEditCC : public CEdit
{
// Construction
public:
	CEditCC() {};

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditCC)
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITCC_H__F25061C6_D99D_49D6_B7FD_253AD1FA29FA__INCLUDED_)
