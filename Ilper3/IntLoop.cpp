//////////////////////////////////////////////////////////////////////
// ILPER 2.50 for Windows
// Copyright (c) 2013-2016  Christoph Giesselink
// Visual C++ MFC Winapp version by Jean-Michel Vansteene 2026
//
// IntLoop.cpp: implementation of the CIntLoop class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "IntLoop.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIntLoop::CIntLoop()
	: m_bLoopClosed(false)
{

}

CIntLoop::CIntLoop(const std::vector<CIL *>& vDevice)
	: m_vDevice(vDevice), m_bLoopClosed(false)
{

}

CIntLoop::CIntLoop(const std::multimap<UINT, CIL *>& mapDevice)
	: m_bLoopClosed(false)
{
	this->SetDevList(mapDevice);
}

VOID CIntLoop::SetDevList(const std::vector<CIL *>& vDevice)
{
	m_vDevice = vDevice;
}

VOID CIntLoop::SetDevList(const std::multimap<UINT, CIL *>& mapDevice)
{
	std::multimap<UINT, CIL *>::const_iterator it;

	m_vDevice.clear();

	for (it = mapDevice.begin(); it != mapDevice.end(); it++)
	{
		if (it->first != 0)					// device enabled
		{
			m_vDevice.push_back(it->second);
		}
	}
}

// --------------------------------
// Device(frame)
//
// transmit the frame to all
// internal virtual devices
// --------------------------------

WORD CIntLoop::Device(WORD wFrame)
{
	std::vector<CIL *>::const_iterator itDevice;

	m_bLoopClosed = true;

	// this is the virtual loop of internal instruments
	for (itDevice = m_vDevice.begin(); m_bLoopClosed && itDevice != m_vDevice.end(); itDevice++)
	{
		wFrame = (*itDevice)->Device(wFrame);
		m_bLoopClosed = (*itDevice)->IsLoopClosed();
	}
	return wFrame;
}
