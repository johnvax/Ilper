//////////////////////////////////////////////////////////////////////
// ILPER 2.50 for Windows
// Copyright (c) 2008-2012  J-F Garnier
// Visual C++ version by Christoph Giesselink 2017
// Visual C++ MFC Winapp version by Jean-Michel Vansteene 2026
//
// EditCC.cpp: implementation of the CEditCC class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "EditCC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MES_CUT			_T("Cu&t\tCtrl+X")
#define MES_COPY		_T("&Copy\tCtrl+C")
#define MES_DELETE		_T("&Delete\tDel")
#define MES_SELECTALL	_T("Select &All\tCtrl+A")
#define ME_SELECTALL	WM_USER + 0x7000

BEGIN_MESSAGE_MAP(CEditCC, CEdit)
	//{{AFX_MSG_MAP(CEditCC)
	ON_WM_CONTEXTMENU()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditCC message handlers

void CEditCC::OnContextMenu(CWnd* pWnd,CPoint point)
{
	SetFocus();
	CMenu menu;
	menu.CreatePopupMenu();

	DWORD sel = GetSel();
	DWORD flags = LOWORD(sel) == HIWORD(sel) ? MF_GRAYED : MF_ENABLED;
	menu.InsertMenu(0, MF_BYPOSITION | flags, WM_CUT, MES_CUT);
	menu.InsertMenu(1, MF_BYPOSITION | flags, WM_COPY, MES_COPY);
	menu.InsertMenu(2, MF_BYPOSITION | flags, WM_CLEAR, MES_DELETE);
	menu.InsertMenu(3, MF_BYPOSITION | MF_SEPARATOR);

	int len = GetWindowTextLength();
	flags = (!len || (LOWORD(sel) == 0 && HIWORD(sel) == len)) ? MF_GRAYED : MF_ENABLED;
	menu.InsertMenu(4, MF_BYPOSITION | flags, ME_SELECTALL, MES_SELECTALL);

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, this);
}

BOOL CEditCC::OnCommand(WPARAM wParam,LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case WM_CUT:
		SetReadOnly(FALSE);
		Cut();
		return SetReadOnly();
	case WM_COPY:
		return (BOOL) SendMessage(LOWORD(wParam));
	case WM_CLEAR:
		SetReadOnly(FALSE);
		Clear();
		return SetReadOnly();
	case ME_SELECTALL:
		return (BOOL) SendMessage(EM_SETSEL,0,-1);
	}
	return CEdit::OnCommand(wParam,lParam);
}

void CEditCC::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (GetKeyState(VK_CONTROL) < 0)		// Ctrl pressed?
	{		
		switch(nChar)
		{
			case 'X':
				PostMessage(WM_COMMAND,WM_CUT,0);
				break;
			case 'C':
			case VK_INSERT:
				PostMessage(WM_COMMAND,WM_COPY,0);
				break;
			case 'A':
				PostMessage(WM_COMMAND,ME_SELECTALL,0);
				break;
		}
	}
	else if (nChar == VK_DELETE)
	{
		PostMessage(WM_COMMAND,WM_CLEAR,0);
	}
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}
