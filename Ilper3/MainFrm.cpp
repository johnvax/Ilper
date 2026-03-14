
//////////////////////////////////////////////////////////////////////
// ILPER 2.50 for Windows
// Copyright (c) 2008-2015  J-F Garnier
// Visual C++ version by Christoph Giesselink 2021
// Visual C++ MFC Winapp version by Jean-Michel Vansteene 2026
//
// // MainFrm.cpp : implementation of the CMainFrame class
//

#include "pch.h"
#include "framework.h"
#include "Ilper.h"

#include "IlSetup.h"
#include "LoopCfg.h"
#include "PilBox.h"
#include "TcpIp.h"
#include "IL.h"
#include "Mnemo.h"
#include "Ildisp.h"
#include "Ildrive.h"
#include "DoslinkDev.h"
#include "IntLoop.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_MESSAGE(WM_INTERFACESTOPPED, OnInterfaceStopped)

    //{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
    ON_WM_SETFOCUS()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_WM_INITMENU()
    ON_WM_CLOSE()
    ON_WM_SETTINGCHANGE()
    ON_BN_CLICKED(IDC_START, OnStart)
    ON_BN_CLICKED(IDC_STOP, OnStop)
    ON_BN_CLICKED(IDC_BROWSE_F1, OnBrowseF1)
    ON_BN_CLICKED(IDC_BROWSE_F2, OnBrowseF2)
    ON_BN_CLICKED(IDC_SCOPE_EN, OnScopeEn)
    ON_BN_CLICKED(IDC_IDY_EN, OnIdyEn)
    ON_BN_CLICKED(IDC_RFC_EN, OnRfcEn)
    ON_COMMAND(IDM_PRTACTLISTEN, OnPrtActListen)
    ON_UPDATE_COMMAND_UI(IDM_PRTACTLISTEN, OnUpdatePrtActListen)
    ON_COMMAND(IDM_HPILSETUP, OnHpilSetup)
    ON_COMMAND(IDM_LOOPCONFIG, OnHpilDeviceInfo)
    ON_BN_CLICKED(IDC_BROWSE_OUT, OnBrowseOut)
    ON_BN_CLICKED(IDC_BROWSE_IN, OnBrowseIn)
    ON_BN_CLICKED(IDC_CLOSEALL, OnCloseall)
//    ON_CBN_SELENDOK(IDC_COMPORT, OnSelendokComport)
    ON_CBN_SELCHANGE(IDC_FONTSIZE, OnSelchangeFontSize)
//    ON_CONTROL(CBN_SELCHANGE, IDC_FONTSIZE, OnSelchangeFontSize)

    ON_COMMAND(IDM_APP_EXIT, &CMainFrame::OnAppExit)
    ON_WM_GETMINMAXINFO()

    //}}AFX_MSG_MAP

END_MESSAGE_MAP()

#define PORT_TCPIP	"TCP/IP"
#define MARGIN      8
#define MINWIDTH    720
#define MINHEIGHT   480

extern CIlperApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame Window

CMainFrame::CMainFrame() noexcept
    :CFrameWnd()
{
    m_bResizing = FALSE;
    m_bInitDone = FALSE;

    m_strAddrOut = _T("localhost");
    m_uPortOut = 60000;
    m_uPortIn = 60001;

    m_dwConnectTimeout = 0xFFFFFFFF;		// client connect timeout in us (default blocked connect timeout)

    m_dwBaudRate = 0;						// auto baudrate
    m_bPilboxIDY = FALSE;					// no PIL-Box IDY support

    m_bScope = FALSE;						// scope disabled
    m_bRFC = TRUE;							// show RFC frames
    m_bIDY = TRUE;							// show IDY frames

    m_strHdiskFile1 = _T("hp9114b.dat");
    m_strHdiskFile2 = _T("hdrive1.dat");

    m_strOutFile = _T("outfile.dat");
    m_strInFile = _T("infile.dat");

    m_bAutoExtAddr = FALSE;					// HP-IL auto extended address support

    m_uPosScope = 10;						// position of scope device in the loop
    m_uPosPrinter = 20;						// position of printer device in the loop
    m_uPosDisk1 = 30;						// position of disk 1 device in the loop
    m_uPosDisk2 = 40;						// position of disk 2 device in the loop
    m_uPosDosLink = 50;						// position of DosLink device in the loop

    m_pComObj = NULL;
    m_pIntLoop = NULL;
    m_pMnemo = NULL;
    m_pPrinter = NULL;
    m_pDiskDrv1 = NULL;
    m_pDiskDrv2 = NULL;
    m_pDosLink = NULL;

//    m_bComEnabled = FALSE;
//    m_bTcpIpEnabled = FALSE;

    m_strComPort = _T("COM4");
}

CMainFrame::~CMainFrame()
{
    delete m_pIntLoop;						// internal HP-IL loop
    delete m_pMnemo;						// the mnemonic disassembler
    delete m_pPrinter;						// the printer
    delete m_pDiskDrv1;						// the disk drive1
    delete m_pDiskDrv2;						// the disk drive2
    delete m_pDosLink;						// the DOSLINK device
    m_fontDisplay.DeleteObject();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    CString strText;
    DWORD dwTracks, dwSides, dwSectors;

    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

 //   SetClassLongPtr(m_hWnd, GCLP_HICON, (LONG_PTR)AfxGetApp()->LoadIcon(IDR_MAINFRAME));
    SetClassLongPtr(m_hWnd, GCLP_HICONSM, (LONG_PTR)AfxGetApp()->LoadIcon(IDR_MAINFRAME));

// Children here
//
// // Mass Storage Group
    strText.LoadString (IDS_STATIC_MASSSTORAGE);
    m_massStorage.Create(strText, WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        CRect(0, 0, 0, 0), this, IDC_STATIC_MASSSTORAGE);

    // Labels & Edits
    strText.LoadString(IDS_DRIVE1);
    m_lblDisk1.Create(L"", WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
        CRect(0, 0, 0, 0), this, IDC_STATIC_DISK1);

    m_CEditDisk1.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
        CRect(0, 0, 0, 0), this, IDC_LIFFILE1);
    m_btnDisk1.Create(L"...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(0, 0, 0, 0), this, IDC_BROWSE_F1);
 
    strText.LoadString(IDS_DRIVE2);
    m_lblDisk2.Create(strText, WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
        CRect(0, 0, 0, 0), this, IDC_STATIC_DISK2);
    m_CEditDisk2.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
        CRect(0, 0, 0, 0), this, IDC_LIFFILE2);
    m_btnDisk2.Create(L"...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(0, 0, 0, 0), this, IDC_BROWSE_F2);

    // Link Combo
    strText.LoadString (IDS_STATIC_PILOBXLINK);
    m_groupLinks.Create(strText, WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        CRect(0, 0, 0, 0), this, 0);
    m_CComboPort.Create(CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
        CRect(0, 0, 0, 0), this, IDC_COMPORT);

    // Boutons Start/Stop
    strText.LoadString (IDS_STATIC_START);
    m_btnStart.Create(strText, WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        CRect(0, 0, 80, 25), this, IDC_START);
    strText.LoadString (IDS_STATIC_STOP);
    m_btnStop.Create(strText, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(0, 0, 80, 25), this, IDC_STOP);

    // Dosklink
    strText.LoadString(IDS_STATIC_DOSLINK);
    m_groupDoslink.Create(strText, WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        CRect(0, 0, 0, 0), this, IDC_STATIC_DOSLINK);
    m_CEditOutFile.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
        CRect(0, 0, 0, 0), this, IDC_OUTFILE);
    m_CEditInFile.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
        CRect(0, 0, 0, 0), this, IDC_INFILE);
    strText.LoadString (IDS_STATIC_OUT);
    m_btnOut.Create(strText, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(0, 0, 0, 0), this, IDC_BROWSE_OUT);
    strText.LoadString (IDS_STATIC_IN);
    m_btnIn.Create(strText, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(0, 0, 0, 0), this, IDC_BROWSE_IN);
    m_btnC.Create(L"C", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CRect(0, 0, 0, 0), this, IDC_CLOSEALL);

    // Printer/Scope
    strText.LoadString (IDS_STATIC_PRINTER);
    m_groupDisplay.Create(strText, WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        CRect(0, 0, 0, 0), this, IDC_STATIC_PRINTER);
    m_editDisplay.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_LEFT | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
        CRect(0, 0, 0, 0), this, IDC_PRINTER);

    strText.LoadString (IDS_STATIC_SCOPE);
    m_groupScope.Create(strText, WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        CRect(0, 0, 0, 0), this, IDC_STATIC_HPIL);
    m_editScope.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_LEFT | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
        CRect(0, 0, 0, 0), this, IDC_SCOPE);

    m_CFontSize.Create(CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
        CRect(0, 0, 0, 0), this, IDC_FONTSIZE);
    int sizes[] = { 50, 75, 100, 125, 150, 200, 300, 400};
    CString str;
    for (int i = 0; i < sizeof(sizes) / sizeof(int); i++) {
        str.Format(_T("%d %%"), sizes[i]);
        m_CFontSize.AddString(str);
    }

    // Bottom label
    str.LoadString (IDS_STATIC_TCPIP_LABEL);
    m_bottomTCPIPLabel.Create(str,
       WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_STATIC_TCPIP_LABEL);
    m_bottomTCPIPPorts.Create(L"",
        WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_STATIC_TCPIP_PORTS);

    // Bottom Scope/IDY/RFC checkboxes
    strText.LoadString (IDS_STATIC_CHK_SCOPE);
    m_chkScope.Create(strText, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        CRect(0, 0, 0, 0), this, IDC_SCOPE_EN);
    m_chkIDY.Create(L"I&DY", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        CRect(0, 0, 0, 0), this, IDC_IDY_EN);
    m_chkRFC.Create(L"&RFC", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        CRect(0, 0, 0, 0), this, IDC_RFC_EN);

    // set default font settings
    NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

    LOGFONT lf = ncm.lfMessageFont;  // System Font

    CDC* pDC = GetDC();
    int lfHeight = abs(lf.lfHeight);
    lf.lfHeight = MulDiv(lfHeight, 120, 100);
    m_fontEdit.CreateFontIndirect(&lf);
    
    lf.lfHeight = MulDiv(abs(lf.lfHeight), 120, 100);
    m_fontSystem.CreateFontIndirect(&lf);

    CWnd* pChild = GetTopWindow();
    while (pChild) {
        if (pChild->IsKindOf(RUNTIME_CLASS(CEdit))) {
            pChild->SetFont(&m_fontEdit);
        }
        else {
            // system size
            pChild->SetFont(&m_fontSystem);
        }

        pChild->SetFont(&m_fontSystem);

        pChild->Invalidate();
        pChild = pChild->GetNextWindow(GW_HWNDNEXT);
    }
    ReleaseDC(pDC);

    // get actual window position
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(wndpl);
    GetWindowPlacement(&wndpl);

     // load positions from registry if available
    m_nPosX = ReadInt(_T("Settings"), _T("WinPosX"), wndpl.rcNormalPosition.left);
    m_nPosY = ReadInt(_T("Settings"), _T("WinPosY"), wndpl.rcNormalPosition.top);
    m_nWidth = ReadInt(_T("Settings"), _T("WinWidth"), (wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left));
    if (m_nWidth < MINWIDTH)
        m_nWidth = MINWIDTH;
    m_nHeight = ReadInt(_T("Settings"), _T("WinHeight"), (wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top));
    if (m_nHeight < MINHEIGHT)
        m_nHeight = MINHEIGHT;

    m_strComPort = ReadString(_T("Settings"), _T("Port"), m_strComPort);
    m_dwBaudRate = ReadInt(_T("Settings"), _T("BaudRate"), m_dwBaudRate);
    m_bPilboxIDY = ReadInt(_T("Settings"), _T("PilboxIDY"), m_bPilboxIDY);

    m_strAddrOut = ReadString(_T("Settings"), _T("AddrOut"), m_strAddrOut);
    m_uPortOut = ReadInt(_T("Settings"), _T("PortOut"), m_uPortOut);
    m_uPortIn = ReadInt(_T("Settings"), _T("PortIn"), m_uPortIn);
    m_dwConnectTimeout = ReadInt(_T("Settings"), _T("ConnectTimeout"), m_dwConnectTimeout);

    m_bAutoExtAddr = ReadInt(_T("Settings"), _T("AutoExtAddr"), m_bAutoExtAddr);

    m_uPosScope = ReadInt(_T("Settings"), _T("LoopPosScope"), m_uPosScope);
    m_uPosPrinter = ReadInt(_T("Settings"), _T("LoopPosPrinter"), m_uPosPrinter);
    m_uPosDisk1 = ReadInt(_T("Settings"), _T("LoopPosDisk1"), m_uPosDisk1);
    m_uPosDisk2 = ReadInt(_T("Settings"), _T("LoopPosDisk2"), m_uPosDisk2);
    m_uPosDosLink = ReadInt(_T("Settings"), _T("LoopPosDosLink"), m_uPosDosLink);

    m_bScope = ReadInt(_T("Scope"), _T("Scope"), m_bScope);
    m_bRFC = ReadInt(_T("Scope"), _T("EnableRFC"), m_bRFC);
    m_bIDY = ReadInt(_T("Scope"), _T("EnableIDY"), m_bIDY);

    // Initialize checkboxes
    m_chkScope.SetCheck(m_bScope);  
    m_chkRFC.SetCheck(m_bRFC);
    m_chkIDY.SetCheck(m_bIDY);

    m_strHdiskFile1 = ReadString(_T("Disk1"), _T("HDFile"), m_strHdiskFile1);
    m_strHdiskFile2 = ReadString(_T("Disk2"), _T("HDFile"), m_strHdiskFile2);
    m_strOutFile = ReadString(_T("DosLink"), _T("FileOut"), m_strOutFile);
    m_strInFile = ReadString(_T("DosLink"), _T("FileIn"), m_strInFile);

    // Set LIF file names
    m_CEditDisk1.SetWindowText(m_strHdiskFile1);
    m_CEditDisk2.SetWindowText(m_strHdiskFile2);

    // DosLink file names
    m_CEditOutFile.SetWindowText(m_strOutFile);
    m_CEditInFile.SetWindowText(m_strInFile);

    // System font (for Printer Edit)
    strText = _T("100 %");
    strText = ReadString(_T("Settings"), _T("FontSize"), strText);
    int index = m_CFontSize.SelectString(-1, strText);
    if (index != CB_ERR) {
        m_CFontSize.SetCurSel(index);
    }
    else {
        m_CFontSize.SetCurSel(2);
    }
//    CFont* pFont = CFont::FromHandle((HFONT)GetStockObject(SYSTEM_FIXED_FONT));
    pDC = m_editDisplay.GetDC();  // ← DC of Printer Edit View
    lf = { 0 };
    _tcscpy_s(lf.lfFaceName, _T("Courier New")); //Segoe UI SemiBold
    lf.lfHeight = -MulDiv(8, pDC->GetDeviceCaps(LOGPIXELSY), 72);
    lf.lfWeight |= FW_BOLD;  // Activate Bold (700)
    lf.lfCharSet = DEFAULT_CHARSET;
    m_fontDisplay.CreateFontIndirect(&lf);
    baseHeight100 = abs(lf.lfHeight);

    m_editDisplay.SetFont(&m_fontDisplay);

    // Licence GPL dans edit gauche
    m_editDisplay.SetWindowTextW(L"This program is free software; you can redistribute it and/or modify it "
        L"under the terms of the GNU General Public License as published by the "
        L"Free Software Foundation; either version 2 of the License, or (at your "
        L"option) any later version.\r\n\r\n"
        L"This program is distributed in the hope that it will be useful, but "
        L"WITHOUT ANY WARRANTY; without even the implied warranty of "
        L"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General "
        L"Public License for more details.");

    // System font (for Scope Edit)
    m_editScope.SetFont(&m_fontDisplay);
    m_editScope.SetWindowTextW(_T(""));		// clear scope display

    // set TCPIP ports label
    strText.Format(_T("In: %d Out: %d"), m_uPortIn, m_uPortOut);
    m_bottomTCPIPPorts.SetWindowTextW(strText);

    SetWindowLocation(m_nPosX, m_nPosY, m_nWidth, m_nHeight);		// set windows location
    ShowScopeWnd(m_bScope);					                        // set state of scope window
    SetCommList(m_strComPort);				                        // update COM port combo box

    bool bAutoExtAddr = (m_bAutoExtAddr == TRUE);

    // initialize the internal instruments
    m_pMnemo = new CMnemo(m_editScope, 30, 5);								// the mnemonic disassembler
    m_pPrinter = new CIldisp(m_editDisplay, 30, bAutoExtAddr);				// the printer
    m_pDiskDrv1 = new CIldrive(m_strHdiskFile1, bAutoExtAddr);				// the disk drive1
    m_pDiskDrv2 = new CIldrive(m_strHdiskFile2, bAutoExtAddr);				// the disk drive2
    m_pDosLink = new CDosLinkDev(m_strOutFile, m_strInFile, bAutoExtAddr);	// the DOSLINK device

    // update scope settings
    m_pMnemo->EnableScope(m_bScope);
    m_pMnemo->EnableIDY(m_bIDY);
    m_pMnemo->EnableRFC(m_bRFC);

    // increase edit box text size to maximum
    m_editScope.SetLimitText(0);
    m_editDisplay.SetLimitText(0);

    if (*theApp.m_szIniFile != 0)			// ini file given
    {
        CString strTitle;

        // new document title
        GetWindowText(strTitle);
        strTitle += CString(_T(" - ")) + theApp.m_lpFilePart;
        SetWindowText(strTitle);	// set document title with ini file
    }
    
    // Set Start/Stop button status
    m_btnStart.EnableWindow(TRUE);
    m_btnStop.EnableWindow(FALSE);
    
    // set new DEVID$ for printer, disk drives and DosLink
    m_pPrinter->SetAID(ReadInt(_T("Printer"), _T("AID"), m_pPrinter->GetAID()));
    m_pPrinter->SetID$(ReadString(_T("Printer"), _T("ID$"), m_pPrinter->GetID$()));
    m_pDiskDrv1->SetID$(ReadString(_T("Disk1"), _T("ID$"), m_pDiskDrv1->GetID$()));
    m_pDiskDrv2->SetID$(ReadString(_T("Disk2"), _T("ID$"), _T("HDRIVE1")));
    m_pDosLink->SetID$(ReadString(_T("DosLink"), _T("ID$"), m_pDosLink->GetID$()));

    // set text with mass storage device names
    strText.LoadString(IDS_DRIVE);
    strText = strText + m_pDiskDrv1->GetID$().SpanExcluding(_T("\\r\\n")).Left(8) + _T(":");
    GetDlgItem(IDC_STATIC_DISK1)->SetWindowText(strText);

    strText.LoadString(IDS_DRIVE);
    strText = strText + m_pDiskDrv2->GetID$().SpanExcluding(_T("\\r\\n")).Left(8) + _T(":");
    GetDlgItem(IDC_STATIC_DISK2)->SetWindowText(strText);

    // set the physical attributes of drive 1
    m_pDiskDrv1->GetPhysicalParameters(dwTracks, dwSides, dwSectors);
    dwTracks = ReadInt(_T("Disk1"), _T("HDTracks"), dwTracks);
    dwSides = ReadInt(_T("Disk1"), _T("HDSides"), dwSides);
    dwSectors = ReadInt(_T("Disk1"), _T("HDSectors"), dwSectors);
    m_pDiskDrv1->SetPhysicalParameters(dwTracks, dwSides, dwSectors);

    // set the physical attributes of drive 2
    dwTracks = ReadInt(_T("Disk2"), _T("HDTracks"), 125);
    dwSides = ReadInt(_T("Disk2"), _T("HDSides"), 8);
    dwSectors = ReadInt(_T("Disk2"), _T("HDSectors"), 64);
    m_pDiskDrv2->SetPhysicalParameters(dwTracks, dwSides, dwSectors);

    // the name of each device
    m_mapNames[m_pMnemo] = _T("Scope");
    m_mapNames[m_pPrinter] = _T("Printer");
    m_mapNames[m_pDiskDrv1] = _T("Drive1");
    m_mapNames[m_pDiskDrv2] = _T("Drive2");
    m_mapNames[m_pDosLink] = _T("DosLink");

    // configure the internal loop
    std::multimap<UINT, CIL*> mapDevice;						// list of devices
    mapDevice.insert(std::make_pair(m_uPosScope, m_pMnemo));		// add IL frame disassembler
    mapDevice.insert(std::make_pair(m_uPosPrinter, m_pPrinter));	// add printer instrument
    mapDevice.insert(std::make_pair(m_uPosDisk1, m_pDiskDrv1));	// add disk drive1 instrument
    mapDevice.insert(std::make_pair(m_uPosDisk2, m_pDiskDrv2));	// add disk drive2 instrument
    mapDevice.insert(std::make_pair(m_uPosDosLink, m_pDosLink));	// add DOSLINK instrument
    m_pIntLoop = new CIntLoop(mapDevice);						// create internal instrument loop class

    // Resize fonts is necessary
    OnSelchangeFontSize();
    m_bInitDone = TRUE;

    // Refresh
    CRect rc;
    GetClientRect(&rc);
    SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.Width(), rc.Height()));

    // press the "Start" button
    if (theApp.m_bStart) PostMessage(WM_COMMAND, IDC_START);

    return 0;
}

////////////////////////////////////////////////////////////////////////
void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
    CFrameWnd::OnSetFocus(pOldWnd);
    // Comme CDialog : focus 1er TABSTOP ou DEFPUSHBUTTON
    m_btnStart.SetFocus();
}

////////////////////////////////////////////////////////////////////////
// Act as a Dialog DEFPUSHBUTTON
BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
        // clic as for a CDialog !
        if (m_pComObj == NULL)
        {
            m_btnStart.SendMessage(BM_CLICK, 0, 0);
            return TRUE;  // Message processed
        }
        else {
            m_btnStop.SendMessage(BM_CLICK, 0, 0);
            return TRUE;  // Message processed
        }
    }
    return CFrameWnd::PreTranslateMessage(pMsg);
}

////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: changez ici la classe ou les styles Window en modifiant
	//  CREATESTRUCT cs
    if (!CFrameWnd::PreCreateWindow(cs))
        return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
    cs.style &= ~WS_BORDER;

    cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS,
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_3DFACE + 1), NULL);

      return TRUE;
}

// diagnostics for CMainFrame

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CIlperDlg message handlers

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC)
{
    if (m_bResizing)
        return TRUE;  // Skip erase during resize
    return CFrameWnd::OnEraseBkgnd(pDC);  // Normal
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
    // cx and cy are client area
    CFrameWnd::OnSize(nType, cx, cy);
    m_bResizing = TRUE;
    SetRedraw(FALSE);
    CRect rect1, rect2;

    int width = cx, height = cy;
    int margin = MARGIN, spacing = MARGIN;
    int nextBottom = 0;

    /* Get Static text width & height */
    CString text;
    m_lblDisk1.GetWindowText(text);
    CDC* pDC = m_lblDisk1.GetDC();
    CFont* pOldFont = pDC->SelectObject(m_lblDisk1.GetFont()); // to get top group size
    CSize size = pDC->GetTextExtent(text);
    m_lblDisk1.ReleaseDC(pDC);

    int textHeight = size.cy + 4;

    // Drive 1
    rect1.left = 2 * margin;
    rect1.top = margin + textHeight;
    rect1.right = rect1.left + size.cx + 6;
    rect1.bottom = rect1.top + textHeight;
    m_lblDisk1.MoveWindow(&rect1, TRUE);

    // Drive 2
    rect2.left = rect1.left;
    rect2.right = rect1.right;
    rect2.top = rect1.bottom + spacing;
    rect2.bottom = rect2.top + textHeight;
    m_lblDisk2.MoveWindow(&rect2, TRUE);

    // Edit 1
    rect1.left = rect1.right + spacing;
    rect1.right = width / 2;
    if ((rect1.right - rect1.left)  < 50) rect1.right = rect1.left + 50;
    m_CEditDisk1.MoveWindow(rect1, TRUE);

    int editHeight = rect1.Height();

    // Button Drive 1
    m_btnDisk1.GetWindowText(text);
    pDC = m_btnDisk1.GetDC();
    pOldFont = pDC->SelectObject(m_btnDisk1.GetFont());
    size = pDC->GetTextExtent(text);
    m_btnDisk1.ReleaseDC(pDC);

    rect1.left = rect1.right + spacing;
    rect1.right = rect1.left + size.cx + 16;
    rect1.bottom = rect1.top + textHeight;
    m_btnDisk1.MoveWindow(rect1, TRUE);

    // Edit 2
    rect2.left = rect2.right + spacing;
    rect2.right = width / 2;
    if ((rect2.right - rect2.left) < 50) rect2.right = rect2.left + 50;
    m_CEditDisk2.MoveWindow(rect2, TRUE);

    // Button Drive 2
    rect2.left = rect1.left;
    rect2.right = rect1.right;
    m_btnDisk2.MoveWindow(rect2, TRUE);

    // Mass Storage Group
    rect1.left = margin;
    rect1.top = margin;
    rect1.bottom = rect2.bottom + spacing;
    rect1.right = rect2.right + spacing;

    m_massStorage.MoveWindow(rect1, TRUE);
    nextBottom = rect1.bottom;

    // Start - Stop buttons
    m_btnStart.GetWindowText(text);
    pDC = m_btnStart.GetDC();
    pOldFont = pDC->SelectObject(m_btnStart.GetFont());

    size = pDC->GetTextExtent(text);
    m_btnStart.ReleaseDC(pDC);

    rect1.right = width - margin - spacing;
    rect1.left = rect1.right - size.cx - 6;
    rect1.top = margin + textHeight;
    rect1.bottom = rect1.top + size.cy + 4;
    m_btnStart.MoveWindow(rect1, TRUE);

    m_btnStop.MoveWindow(rect1, TRUE);
    rect2.left = rect1.left;
    rect2.top = rect1.bottom + spacing;
    rect2.right = rect1.right;
    rect2.bottom = rect2.top + size.cy + 4;
    m_btnStop.MoveWindow(rect2, TRUE);

    // Link Combo
    int nScrollWidth = ::GetSystemMetrics(SM_CXVSCROLL);
    int nNumEntries = m_CComboPort.GetCount();
    pDC = m_CComboPort.GetDC();
    pOldFont = pDC->SelectObject(m_CComboPort.GetFont());
    int nWidth = 0;
    for (int i = 0; i < nNumEntries; i++)
    {
        m_CComboPort.GetLBText(i, text);
        int nLength = pDC->GetTextExtent(text).cx + nScrollWidth;
        nWidth = max(nWidth, nLength);
    }
    // Add margin space to the calculations
    nWidth += pDC->GetTextExtent(L"0").cx;
    m_CComboPort.ReleaseDC(pDC);

    rect1.left -= (nWidth + (4 * spacing) + 6);
    rect1.right = rect1.left + nWidth + (2 * spacing);
    rect1.top = margin + textHeight;
    rect1.bottom = rect1.top + rect2.Height();
    m_CComboPort.MoveWindow(rect1, TRUE);

    // Link Group
    rect2.left = rect1.left - spacing;
    rect2.top = rect1.top - textHeight;
    rect2.bottom = rect1.bottom + spacing;
    nextBottom = max(nextBottom, rect2.bottom);
    rect2.bottom = nextBottom;
    rect2.right = rect1.right + spacing;
    m_groupLinks.MoveWindow(rect2, TRUE);

    // Doslink Group
    rect1.left = margin;
    rect1.right = cx - margin;
    rect1.top = nextBottom + spacing;
    rect1.bottom = rect1.top + (2 * textHeight) + spacing;

    m_groupDoslink.MoveWindow(rect1, TRUE);
    nextBottom = rect1.bottom;

    // File Out
    rect1.left += spacing;
    rect1.top += textHeight ;
    rect1.bottom = rect1.top + textHeight;
    rect1.right = rect1.left + (width / 3);
    if ((rect1.right - rect1.left) < 50) rect1.right = rect1.left + 50;

    m_CEditOutFile.MoveWindow(rect1, TRUE);

    // Button Out
    m_btnOut.GetWindowText(text);
    pDC = m_btnDisk1.GetDC();
    pOldFont = pDC->SelectObject(m_btnOut.GetFont());
    size = pDC->GetTextExtent(text);
    m_btnOut.ReleaseDC(pDC);

    rect1.left = rect1.right + spacing;
    rect1.right = rect1.left + size.cx + 16;
    rect1.bottom = rect1.top + textHeight;
    m_btnOut.MoveWindow(rect1, TRUE);

    // FIle In
    rect1.left = rect1.right + spacing;
    rect1.right = rect1.left + (width / 3);
    if ((rect1.right - rect1.left) < 50) rect1.right = rect1.left + 50;

    m_CEditInFile.MoveWindow(rect1, TRUE);

    // Button In
    rect1.left = rect1.right + spacing;
    rect1.right = rect1.left + size.cx + 16;
    rect1.bottom = rect1.top + textHeight;
    m_btnIn.MoveWindow(rect1, TRUE);

    // Buttun C (Same Size as Out button)
    rect1.right = cx - margin;
    rect1.left = rect1.right - size.cx - 16;
    m_btnC.MoveWindow(rect1, TRUE);

    // Bottom Bar
    // Font Size Combo

    pDC = m_CFontSize.GetDC();
    pOldFont = pDC->SelectObject(m_CFontSize.GetFont());
    nWidth = 0;
    for (int i = 0; i < nNumEntries; i++)
    {
        m_CFontSize.GetLBText(i, text);
        int nLength = pDC->GetTextExtent(text).cx + nScrollWidth;
        nWidth = max(nWidth, nLength);
    }
    // Add margin space to the calculations
    nWidth += pDC->GetTextExtent(L"0").cx;
    m_CFontSize.ReleaseDC(pDC);

    rect1.left = margin;
    rect1.right = rect1.left + nWidth + (2 * spacing);
    rect1.top = height - margin - spacing - textHeight;
    rect1.bottom = height - margin - spacing;
    m_CFontSize.MoveWindow(rect1, TRUE);

    // Label TCPIP ports
    m_bottomTCPIPLabel.GetWindowText(text);
    pDC = m_bottomTCPIPLabel.GetDC();
    pOldFont = pDC->SelectObject(m_bottomTCPIPLabel.GetFont()); // to get top group size
    size = pDC->GetTextExtent(text);
    m_bottomTCPIPLabel.ReleaseDC(pDC);
    
    rect1.left = rect1.right + spacing;
    rect1.right = rect1.left + size.cx + 6;
    m_bottomTCPIPLabel.MoveWindow(rect1, TRUE);

    m_bottomTCPIPPorts.GetWindowText(text);
    pDC = m_bottomTCPIPPorts.GetDC();
    pOldFont = pDC->SelectObject(m_bottomTCPIPPorts.GetFont()); // to get top group size
    size = pDC->GetTextExtent(text);
    m_bottomTCPIPPorts.ReleaseDC(pDC);

    rect1.left = rect1.right + spacing;
    rect1.right = rect1.left + size.cx + 6;
    m_bottomTCPIPPorts.MoveWindow(rect1, TRUE);

    // Checkboxes from right to left
    // [X]  RFC
    nWidth = GetSystemMetrics(SM_CXMENUCHECK);

    m_chkRFC.GetWindowText(text);
    pDC = m_chkRFC.GetDC();
    pOldFont = pDC->SelectObject(m_chkRFC.GetFont()); // to get top group size
    size = pDC->GetTextExtent(text);
    m_chkRFC.ReleaseDC(pDC);

    rect1.right = width - margin;
    rect1.left = rect1.right - (nWidth + size.cx + 6);
    m_chkRFC.MoveWindow(rect1, TRUE);

    // [X] IDY 
    m_chkIDY.GetWindowText(text);
    pDC = m_chkIDY.GetDC();
    pOldFont = pDC->SelectObject(m_chkIDY.GetFont()); // to get top group size
    size = pDC->GetTextExtent(text);
    m_chkIDY.ReleaseDC(pDC);

    rect1.right = rect1.left - spacing;
    rect1.left = rect1.right - (nWidth + size.cx + 6);
    m_chkIDY.MoveWindow(rect1, TRUE);

    // [X] Scope
    m_chkScope.GetWindowText(text);
    pDC = m_chkScope.GetDC();
    pOldFont = pDC->SelectObject(m_chkScope.GetFont()); // to get top group size
    size = pDC->GetTextExtent(text);
    m_chkScope.ReleaseDC(pDC);

    rect1.right = rect1.left - spacing;
    rect1.left = rect1.right - (nWidth + size.cx + 6);
    m_chkScope.MoveWindow(rect1, TRUE);

    // And now : edit boxes
    if (m_bScope) {
        rect2.bottom = rect1.bottom = rect1.top - spacing;
        rect2.top = rect1.top = nextBottom + spacing;
        rect1.left = margin;
        rect1.right = (width / 2) - spacing;

        rect2.left = rect1.right + spacing;
        rect2.right = width - spacing;

        m_groupDisplay.MoveWindow(rect1, TRUE);
        rect1.left += spacing;
        rect1.right -= spacing;
        rect1.top += editHeight;
        rect1.bottom -= spacing;
        m_editDisplay.MoveWindow(rect1, TRUE);

        m_groupScope.MoveWindow(rect2, TRUE);
        rect2.left += spacing;
        rect2.right -= spacing;
        rect2.top += editHeight;
        rect2.bottom -= spacing;
        m_editScope.MoveWindow(rect2, TRUE);
    }
    else {
        rect1.bottom = rect1.top - spacing;
        rect1.top = nextBottom + spacing;
        rect1.left = margin;
        rect1.right = width - margin;
        m_groupDisplay.MoveWindow(rect1, TRUE);

        rect1.left += spacing;
        rect1.right -= spacing;
        rect1.top += editHeight;
        rect1.bottom -= spacing;
        m_editDisplay.MoveWindow(rect1, TRUE);
    }

    // Set LIF file names
    m_CEditDisk1.SetWindowText(m_strHdiskFile1);
    m_CEditDisk1.SetSel(0, -1);				// scroll to end of LIF file path

    m_CEditDisk2.SetWindowText(m_strHdiskFile2);
    m_CEditDisk2.SetSel(0, -1);				// scroll to end of LIF file path

    // DosLink file names
    m_CEditOutFile.SetWindowText(m_strOutFile);
    m_CEditOutFile.SetSel(0, -1);			// scroll to end of out file path

    m_CEditInFile.SetWindowText(m_strInFile);
    m_CEditInFile.SetSel(0, -1);			// scroll to end of in file path

    m_bResizing = FALSE;
    SetRedraw(TRUE);
    Invalidate();
}

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
     return CFrameWnd::OnCommand(wParam, lParam);
}

void CMainFrame::OnInitMenu(CMenu* pMenu)
{
    pMenu->CheckMenuItem(IDM_PRTACTLISTEN, m_pPrinter->IsActiveListener() ? MF_CHECKED : MF_UNCHECKED);
}

// Application minimal size
void CMainFrame::OnGetMinMaxInfo(MINMAXINFO * lpMMI)
{
    CFrameWnd::OnGetMinMaxInfo(lpMMI);

    lpMMI->ptMinTrackSize.x = MINWIDTH;
    lpMMI->ptMinTrackSize.y = MINHEIGHT;
}

void CMainFrame::OnStart()
{
    // TODO: Add your control notification handler code here
     SetDlgItemText(IDC_PRINTER, _T(""));		// clear printer display

    // get LIF file names
    m_CEditDisk1.GetWindowText(m_strHdiskFile1);
    m_CEditDisk1.SetSel(0, -1);				// scroll to end of LIF file path
    m_pDiskDrv1->SetHdisc(m_strHdiskFile1);	// set file name for disk drive 1

    m_CEditDisk2.GetWindowText(m_strHdiskFile2);
    m_CEditDisk2.SetSel(0, -1);				// scroll to end of LIF file path
    m_pDiskDrv2->SetHdisc(m_strHdiskFile2);	// set file name for disk drive 2

    // get DosLink file names
    m_CEditOutFile.GetWindowText(m_strOutFile);
    m_CEditOutFile.SetSel(0, -1);			// scroll to end of out file path
    m_pDosLink->SetOutFile(m_strOutFile);	// set file name for DosLink out

    m_CEditInFile.GetWindowText(m_strInFile);
    m_CEditInFile.SetSel(0, -1);				// scroll to end of in file path
    m_pDosLink->SetInFile(m_strInFile);		// set file name for DosLink in

    // get COM port setting from combo box
    m_CComboPort.GetWindowText(m_strComPort);

    if (m_strComPort != _T(PORT_TCPIP))
    {
        m_pComObj = new CPilbox(m_pIntLoop, m_strComPort, m_dwBaudRate, m_bPilboxIDY != 0, this);
    }
    else
    {
        m_pComObj = new CTcpIp(m_pIntLoop, m_strAddrOut, m_uPortOut, m_uPortIn, m_dwConnectTimeout, this);
    }

    if (m_pComObj != NULL)
    {
        if (m_pComObj->StartServer() == true) // init server
        {
            m_CComboPort.EnableWindow(FALSE);
            m_CEditDisk1.EnableWindow(FALSE);
            m_CEditDisk2.EnableWindow(FALSE);
            m_CEditOutFile.EnableWindow(FALSE);
            m_CEditInFile.EnableWindow(FALSE);
            m_btnStart.SetButtonStyle(BS_PUSHBUTTON);
            m_btnStart.EnableWindow(FALSE);
            m_btnStop.EnableWindow(TRUE);
            m_btnStop.SetButtonStyle(BS_DEFPUSHBUTTON);
            m_btnStop.SetFocus();
        }
        else								// error
        {
            ASSERT(m_pComObj);				// free Interface class
            delete m_pComObj;
            m_pComObj = NULL;
        }
    }
}

void CMainFrame::OnStop()
{
    // TODO: Add your control notification handler code here
    ASSERT(m_pComObj);
    m_pComObj->StopServer();                            // quit server thread
}

LRESULT CMainFrame::OnInterfaceStopped(WPARAM wParam, LPARAM lParam)
{
    // quit message from Interface worker thread
    CInterface* pComObj = reinterpret_cast<CInterface*>(wParam);
    ASSERT(pComObj == m_pComObj);

    ASSERT(m_pComObj);
    delete m_pComObj;
    m_pComObj = NULL;

	m_CComboPort.EnableWindow();
	m_CEditDisk1.EnableWindow();
	m_CEditDisk2.EnableWindow();
	m_CEditOutFile.EnableWindow();
	m_CEditInFile.EnableWindow();
	m_btnStop.SetButtonStyle(BS_PUSHBUTTON);
	m_btnStop.EnableWindow(FALSE);
	m_btnStart.EnableWindow(TRUE);
	m_btnStart.SetButtonStyle(BS_DEFPUSHBUTTON);
	m_btnStart.SetFocus();
    return 0;
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
}

/////////////////////////////////////////////////////////////////////////////
// CIlperDlg settings helper functions

CString CMainFrame::ReadString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
    if (*theApp.m_szIniFile != 0)
    {
        TCHAR cBuffer[1024];

        ::GetPrivateProfileString(lpszSection, lpszEntry, lpszDefault, cBuffer, ARRAYSIZEOF(cBuffer), theApp.m_szIniFile);
        return CString(cBuffer);
    }
    else
    {
        return AfxGetApp()->GetProfileString(lpszSection, lpszEntry, lpszDefault);
    }
}

UINT CMainFrame::ReadInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, INT nDefault)
{
    if (*theApp.m_szIniFile != 0)
    {
        return ::GetPrivateProfileInt(lpszSection, lpszEntry, nDefault, theApp.m_szIniFile);
    }
    else
    {
        return AfxGetApp()->GetProfileInt(lpszSection, lpszEntry, nDefault);
    }
}

BOOL CMainFrame::WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
    if (*theApp.m_szIniFile != 0)
    {
        return ::WritePrivateProfileString(lpszSection, lpszEntry, lpszValue, theApp.m_szIniFile);
    }
    else
    {
        return AfxGetApp()->WriteProfileString(lpszSection, lpszEntry, lpszValue);
    }
}

BOOL CMainFrame::WriteInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, INT nValue)
{
    if (*theApp.m_szIniFile != 0)
    {
        TCHAR s[16];
        wsprintf(s, _T("%u"), nValue);
        return ::WritePrivateProfileString(lpszSection, lpszEntry, s, theApp.m_szIniFile);
    }
    else
    {
        return AfxGetApp()->WriteProfileInt(lpszSection, lpszEntry, nValue);
    }
}

void CMainFrame::OnSelendokComport()
{
    // TODO: Add your control notification handler code here
    m_CComboPort.GetWindowText(m_strComPort);
}

void CMainFrame::OnBrowseF1()
{
    // TODO: Add your control notification handler code here
    m_CEditDisk1.GetWindowText(m_strHdiskFile1);
    CFileDialog dlg(
        TRUE,
        NULL,
        m_strHdiskFile1,
        OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
        _T("LIF Data Files (*.dat;*.lif)|*.dat;*.lif|All Files (*.*)|*.*||"));
    if (IDOK == dlg.DoModal())
    {
        m_strHdiskFile1 = dlg.GetPathName();
        m_CEditDisk1.SetWindowText(m_strHdiskFile1);
        m_CEditDisk1.SetSel(0, -1);			// scroll to end of LIF file path

        // set file name for disk drive
        m_pDiskDrv1->SetHdisc(m_strHdiskFile1);
    }
}

void CMainFrame::OnBrowseF2()
{
    // TODO: Add your control notification handler code here
    m_CEditDisk2.GetWindowText(m_strHdiskFile2);
    CFileDialog dlg(
        TRUE,
        NULL,
        m_strHdiskFile2,
        OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
        _T("LIF Data Files (*.dat;*.lif)|*.dat;*.lif|All Files (*.*)|*.*||"));
    if (IDOK == dlg.DoModal())
    {
        m_strHdiskFile2 = dlg.GetPathName();
        m_CEditDisk2.SetWindowText(m_strHdiskFile2);
        m_CEditDisk2.SetSel(0, -1);			// scroll to end of LIF file path

        // set file name for disk drive
        m_pDiskDrv2->SetHdisc(m_strHdiskFile2);
    }
}

// set window location
VOID CMainFrame::SetWindowLocation(INT nPosX, INT nPosY, INT nWidth, INT nHeight)
{
    WINDOWPLACEMENT wndpl;
    RECT* pRc = &wndpl.rcNormalPosition;

    wndpl.length = sizeof(wndpl);
    GetWindowPlacement(&wndpl);
    pRc->right = nWidth /* pRc->right - pRc->left */ + nPosX;
    pRc->bottom = nHeight /* pRc->bottom - pRc->top */ + nPosY;
    pRc->left = nPosX;
    pRc->top = nPosY;
    SetWindowPlacement(&wndpl);
    return;
}

// set listfield for serial combo box
VOID CMainFrame::SetCommList(LPCTSTR szComPort)
{
    INT  nSelect, nSelIndex;
    HKEY hKey;

    m_CComboPort.ResetContent();			// delete current list

    nSelect = 0;							// set selection to 1st one
    nSelIndex = 0;							// preset selector

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        _T("Hardware\\DeviceMap\\SerialComm"),
        0,
        KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
        &hKey) == ERROR_SUCCESS)
    {
        TCHAR cKey[256], cData[256];
        DWORD dwKeySize, dwDataSize;
        bool  bFound;
        DWORD dwType, dwEnumVal;
        LONG  lRet;

        bFound = false;						// preset not found

        dwEnumVal = 0;
        do
        {
            dwKeySize = ARRAYSIZEOF(cKey);	// init return buffer sizes
            dwDataSize = sizeof(cData);

            lRet = RegEnumValue(hKey, dwEnumVal++,
                cKey, &dwKeySize,
                NULL, &dwType,
                reinterpret_cast<LPBYTE>(cData), &dwDataSize);

            if (lRet == ERROR_SUCCESS && dwType == REG_SZ)
            {
                if (lstrcmp(cData, szComPort) == 0)
                {
                    nSelect = nSelIndex;
                    bFound = true;
                }

                m_CComboPort.AddString(cData);
                ++nSelIndex;
            }
        } while (lRet == ERROR_SUCCESS);
        _ASSERT(lRet == ERROR_NO_MORE_ITEMS);
        RegCloseKey(hKey);

        // port doesn't exists any more and it's not TCP/IP
        if (!bFound && lstrcmp(_T(PORT_TCPIP), szComPort) != 0)
        {
            // add it anyway
            m_CComboPort.AddString(szComPort);
            nSelect = nSelIndex;
        }
    }

    // add TCP/IP setting
    LPCTSTR lpszItem = _T(PORT_TCPIP);
    m_CComboPort.AddString(lpszItem);
    if (lstrcmp(lpszItem, szComPort) == 0)
    {
        nSelect = nSelIndex;
    }

    m_CComboPort.SetCurSel(nSelect);		// set cursor
    return;
}


// SCOPE Window
// set state of scope window
VOID CMainFrame::ShowScopeWnd(BOOL bEnable)
{
    CWnd* p;
    int   nCmdShow;

    CRect rcWnd;
    GetClientRect(&rcWnd);

    nCmdShow = (bEnable ? SW_SHOW : SW_HIDE);

    // enable/disable the scope window
    m_groupScope.ShowWindow(nCmdShow);
    m_editScope.ShowWindow(nCmdShow);

    // enable/disable the IDY and RFC check boxes
    p = GetDescendantWindow(IDC_IDY_EN);
    p->EnableWindow(bEnable);				// necessary to disable the hotkey
    p->ShowWindow(nCmdShow);				// when box is hidden
    p = GetDescendantWindow(IDC_RFC_EN);
    p->EnableWindow(bEnable);
    p->ShowWindow(nCmdShow);

    SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rcWnd.Width(), rcWnd.Height()));
    return;
}

void CMainFrame::OnBrowseOut()
{
    // TODO: Add your control notification handler code here
    m_CEditOutFile.GetWindowText(m_strOutFile);
    CFileDialog dlg(
        TRUE,
        NULL,
        m_strOutFile,
        OFN_PATHMUSTEXIST,
        _T("Data Files (*.dat)|*.dat|All Files (*.*)|*.*||"));

    if (IDOK == dlg.DoModal())
    {
        m_strOutFile = dlg.GetPathName();
        m_CEditOutFile.SetWindowText(m_strOutFile);
        m_CEditOutFile.SetSel(0, -1);		// scroll to end of out file path

        // set file name for out file
        m_pDosLink->SetOutFile(m_strOutFile);
    }
}

void CMainFrame::OnBrowseIn()
{
    // TODO: Add your control notification handler code here
    m_CEditInFile.GetWindowText(m_strInFile);
    CFileDialog dlg(
        TRUE,
        NULL,
        m_strInFile,
        OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
        _T("Data Files (*.dat)|*.dat|All Files (*.*)|*.*||"));

    if (IDOK == dlg.DoModal())
    {
        m_strInFile = dlg.GetPathName();
        m_CEditInFile.SetWindowText(m_strInFile);
        m_CEditInFile.SetSel(0, -1);			// scroll to end of in file path

        // set file name for out file
        m_pDosLink->SetInFile(m_strInFile);
    }
}

void CMainFrame::OnCloseall()
{
    // TODO: Add your control notification handler code here
    m_pDosLink->ClearDevice();
}


void CMainFrame::OnScopeEn()
{
    // TODO: Add your control notification handler code here
    m_bScope = (m_chkScope.GetCheck() == BST_CHECKED);  // update m_bScope variable
    m_pMnemo->EnableScope(m_bScope);
    ShowScopeWnd(m_bScope);					// set state of scope window
    return;
}

void CMainFrame::OnIdyEn()
{
    // TODO: Add your control notification handler code here
    m_bIDY = (m_chkIDY.GetCheck() == BST_CHECKED);  // update m_bIDY variable
    m_pMnemo->EnableIDY(m_bIDY);
    return;
}

void CMainFrame::OnRfcEn()
{
    // TODO: Add your control notification handler code here
    m_bRFC = (m_chkRFC.GetCheck() == BST_CHECKED);  // update m_bRFC variable
    m_pMnemo->EnableRFC(m_bRFC);
    return;
}

// File > Printer as Acgtive Listener
void CMainFrame::OnPrtActListen()
{
    CMenu* pMenu = GetMenu();
    
    // get toggled menu item state
    UINT nState = pMenu->GetMenuState(IDM_PRTACTLISTEN, MF_BYCOMMAND) ^ MF_CHECKED;

    // enable/disable to active listener mode
    m_pPrinter->SetActiveListener(nState != 0);

    pMenu->CheckMenuItem(IDM_PRTACTLISTEN, m_pPrinter->IsActiveListener() ? MF_CHECKED : MF_UNCHECKED);
}

// File > TCPIP Setup
void CMainFrame::OnHpilSetup()
{
    CString strText;

    CILSetup dlg;
    dlg.m_strAddrOut = m_strAddrOut;
    dlg.m_uPortOut = m_uPortOut;
    dlg.m_uPortIn = m_uPortIn;
    if (IDOK == dlg.DoModal())
    {
        m_strAddrOut = dlg.m_strAddrOut;
        m_uPortOut = dlg.m_uPortOut;
        m_uPortIn = dlg.m_uPortIn;
        // Setbottom TCPIP Ports Label
        strText.Format(_T("In: %d Out: %d"), m_uPortIn, m_uPortOut);
        m_bottomTCPIPPorts.SetWindowTextW(strText);
    }
}

// File > HPIL Device Info
void CMainFrame::OnHpilDeviceInfo()
{
    CLoopCfg(m_mapNames, m_pIntLoop->GetDevList()).DoModal();
}

void CMainFrame::OnUpdatePrtActListen(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_pComObj ? FALSE : TRUE);  // true to activate, false to deactivate
}

void CMainFrame::OnAppExit()
{
    PostMessage(WM_CLOSE);                  // Properly Quit
}

void CMainFrame::OnClose()
{
    // TODO: Add extra cleanup here
    if (m_pComObj != NULL)					// Interface running
    {
        OnStop();
        PostMessage(WM_CLOSE);	            // call myself, until thread finished
        return;
    }

    // get actual window position
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(wndpl);
    GetWindowPlacement(&wndpl);
    m_nPosX = wndpl.rcNormalPosition.left;
    m_nPosY = wndpl.rcNormalPosition.top;
    m_nWidth = (wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left);
    m_nHeight = (wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top);

    // get LIF file names
    m_CEditDisk1.GetWindowText(m_strHdiskFile1);
    m_CEditDisk2.GetWindowText(m_strHdiskFile2);

    // get DosLink file names
    m_CEditOutFile.GetWindowText(m_strOutFile);
    m_CEditInFile.GetWindowText(m_strInFile);

    // get COM port setting from combo box
    m_CComboPort.GetWindowText(m_strComPort);

    TCHAR szAppFile[_MAX_PATH];				// full pathname of application
    GetModuleFileName(NULL, szAppFile, ARRAYSIZEOF(szAppFile));
    WriteString(_T("Run"), _T("App"), szAppFile);

    WriteInt(_T("Settings"), _T("WinPosX"), m_nPosX);
    WriteInt(_T("Settings"), _T("WinPosY"), m_nPosY);
    WriteInt(_T("Settings"), _T("WinWidth"), m_nWidth);
    WriteInt(_T("Settings"), _T("WinHeight"), m_nHeight);

    WriteString(_T("Settings"), _T("Port"), m_strComPort);
    WriteInt(_T("Settings"), _T("BaudRate"), m_dwBaudRate);
    WriteInt(_T("Settings"), _T("PilboxIDY"), m_bPilboxIDY);
    WriteString(_T("Settings"), _T("AddrOut"), m_strAddrOut);
    WriteInt(_T("Settings"), _T("PortOut"), m_uPortOut);
    WriteInt(_T("Settings"), _T("PortIn"), m_uPortIn);
    WriteInt(_T("Settings"), _T("ConnectTimeout"), m_dwConnectTimeout);

    WriteInt(_T("Settings"), _T("AutoExtAddr"), m_bAutoExtAddr);

    WriteInt(_T("Settings"), _T("LoopPosScope"), m_uPosScope);
    WriteInt(_T("Settings"), _T("LoopPosPrinter"), m_uPosPrinter);
    WriteInt(_T("Settings"), _T("LoopPosDisk1"), m_uPosDisk1);
    WriteInt(_T("Settings"), _T("LoopPosDisk2"), m_uPosDisk2);
    WriteInt(_T("Settings"), _T("LoopPosDosLink"), m_uPosDosLink);

    WriteInt(_T("Scope"), _T("Scope"), m_bScope);
    WriteInt(_T("Scope"), _T("EnableRFC"), m_bRFC);
    WriteInt(_T("Scope"), _T("EnableIDY"), m_bIDY);

    WriteInt(_T("Printer"), _T("AID"), m_pPrinter->GetAID());
    WriteString(_T("Printer"), _T("ID$"), m_pPrinter->GetID$());

    WriteString(_T("Disk1"), _T("ID$"), m_pDiskDrv1->GetID$());
    WriteString(_T("Disk1"), _T("HDFile"), m_strHdiskFile1);

    WriteString(_T("Disk2"), _T("ID$"), m_pDiskDrv2->GetID$());
    WriteString(_T("Disk2"), _T("HDFile"), m_strHdiskFile2);

    WriteString(_T("DosLink"), _T("ID$"), m_pDosLink->GetID$());
    WriteString(_T("DosLink"), _T("FileOut"), m_strOutFile);
    WriteString(_T("DosLink"), _T("FileIn"), m_strInFile);

    // get Font Percent
    CString strText;
    m_CFontSize.GetWindowText(strText);
    WriteString(_T("Settings"), _T("FontSize"), strText);
    
    DWORD dwTracks, dwSides, dwSectors;

    // get the physical attributes of drive 1
    m_pDiskDrv1->GetPhysicalParameters(dwTracks, dwSides, dwSectors);
    WriteInt(_T("Disk1"), _T("HDTracks"), dwTracks);
    WriteInt(_T("Disk1"), _T("HDSides"), dwSides);
    WriteInt(_T("Disk1"), _T("HDSectors"), dwSectors);

    // get the physical attributes of drive 2
    m_pDiskDrv2->GetPhysicalParameters(dwTracks, dwSides, dwSectors);
    WriteInt(_T("Disk2"), _T("HDTracks"), dwTracks);
    WriteInt(_T("Disk2"), _T("HDSides"), dwSides);
    WriteInt(_T("Disk2"), _T("HDSectors"), dwSectors);

    CFrameWnd::OnClose();
}

void CMainFrame::OnSelchangeFontSize()
{
    int index = m_CFontSize.GetCurSel();
    if (index == CB_ERR) return;

    CString strPercent;
    m_CFontSize.GetLBText(index, strPercent);
    int percent = _ttoi(strPercent.Left(strPercent.GetLength() - 1));

    // Police de base
    CFont* pBaseFont = m_editDisplay.GetFont();
    LOGFONT baseLf;
    if (pBaseFont && pBaseFont->GetLogFont(&baseLf)) {
//        int baseHeight = abs(baseLf.lfHeight);
        int newHeight = MulDiv(baseHeight100, percent, 100);
        baseLf.lfHeight = -newHeight;

        m_fontDisplay.DeleteObject();
        m_fontDisplay.CreateFontIndirect(&baseLf);
        m_editDisplay.SendMessage(WM_SETFONT, (WPARAM)m_fontDisplay.GetSafeHandle(), TRUE);
        m_editDisplay.Invalidate();

        m_editScope.SendMessage(WM_SETFONT, (WPARAM)m_fontDisplay.GetSafeHandle(), TRUE);
        m_editScope.Invalidate();
    }
}