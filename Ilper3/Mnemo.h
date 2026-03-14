// Mnemo.h: interface for the CMnemo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MNEMO_H__5182FDD7_DF1E_4A5F_BC87_9FAD16A48214__INCLUDED_)
#define AFX_MNEMO_H__5182FDD7_DF1E_4A5F_BC87_9FAD16A48214__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TextSvr.h"
#include "IL.h"

class CMnemo : public CIL
{
public:
	CMnemo(CWnd& wndScope, DWORD dwRefresh = 0, INT nColMax = 3);
	virtual ~CMnemo();

	VOID EnableScope(BOOL bEnable) { m_bScope = bEnable; }
	VOID EnableRFC(BOOL bEnable)   { m_bRFC = bEnable; }
	VOID EnableIDY(BOOL bEnable)   { m_bIDY = bEnable; }

	WORD Device(WORD wFrame) override;

private:
	CTextSvr m_TxtSvr;						// text server
	BOOL m_bScope;							// scope enabled
	BOOL m_bRFC;							// RFC frames enabled
	BOOL m_bIDY;							// IDY frames enabled

	const INT m_nColMax;					// max columns per row

	static VOID ILMnemo(WORD wFrame,LPTSTR s);
	VOID DisplayMnemo(LPCTSTR lpszText);
	VOID DisplayMnemoDbg(LPCTSTR lpszText);
};

#endif // !defined(AFX_MNEMO_H__5182FDD7_DF1E_4A5F_BC87_9FAD16A48214__INCLUDED_)
