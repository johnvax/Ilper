// IntLoop.h: interface for the CIntLoop class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTLOOP_H__7B5D5EC7_C9B6_421A_A9A5_0BECF12F8899__INCLUDED_)
#define AFX_INTLOOP_H__7B5D5EC7_C9B6_421A_A9A5_0BECF12F8899__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IL.h"

class CIntLoop : public CIL
{
public:
	CIntLoop();
	CIntLoop(const std::vector<CIL *>& vDevice);
	CIntLoop(const std::multimap<UINT, CIL *>& mapDevice);

	VOID SetDevList(const std::vector<CIL *>& vDevice);
	VOID SetDevList(const std::multimap<UINT, CIL *>& mapDevice);

	const std::vector<CIL *>& GetDevList() const { return m_vDevice; }

	WORD Device(WORD wFrame) override;
	bool IsLoopClosed() const override { return m_bLoopClosed; }

private:
	bool m_bLoopClosed;						// interface loop is closed

	std::vector<CIL *> m_vDevice;			// list of devices
};

#endif // !defined(AFX_INTLOOP_H__7B5D5EC7_C9B6_421A_A9A5_0BECF12F8899__INCLUDED_)
