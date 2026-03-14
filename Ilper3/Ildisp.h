// Ildisp.h: interface for the CIldisp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ILDISP_H__63B0BA9F_C71A_4323_B0CA_91A0857460C6__INCLUDED_)
#define AFX_ILDISP_H__63B0BA9F_C71A_4323_B0CA_91A0857460C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TextSvr.h"
#include "IldevBase.h"

class CIldisp : public CIldevBase
{
public:
	CIldisp(CWnd& wndDisplay, DWORD dwRefresh = 0, bool bAutoExtAddr = false);
	virtual ~CIldisp();

private:
	CTextSvr m_TxtSvr;						// text server

	bool m_bFesc;							// escape sequence

	VOID DisplayStr(TCHAR c);

	VOID ClearDevice() override;			// clear device
	VOID InData(WORD wFrame) override;		// listener transfer
};

#endif // !defined(AFX_ILDISP_H__63B0BA9F_C71A_4323_B0CA_91A0857460C6__INCLUDED_)
