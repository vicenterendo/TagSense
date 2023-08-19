// HoldingListPlugin.h : main header file for the HoldingListPlugin DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWatchYoJetPluginApp
// See HoldingListPlugin.cpp for the implementation of this class
//

class CWatchYoJetPluginApp : public CWinApp
{
public:
	CWatchYoJetPluginApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
