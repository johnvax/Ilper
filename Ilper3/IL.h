// IL.h: interface for the CIL class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IL_H__40EA2E03_4763_4334_B5FE_CF1E38636032__INCLUDED_)
#define AFX_IL_H__40EA2E03_4763_4334_B5FE_CF1E38636032__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIL
{
public:
	CIL() { }
	virtual ~CIL() { }

	// the device
	virtual WORD Device(WORD wFrame) = 0;
	virtual bool IsLoopClosed() const { return true; }
};

#endif // !defined(AFX_IL_H__40EA2E03_4763_4334_B5FE_CF1E38636032__INCLUDED_)
