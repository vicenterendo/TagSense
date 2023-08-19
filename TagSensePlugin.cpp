// HoldingListPlugin.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "TagSensePlugin.h"
#include "TagSensePlugin2.h"
#include "EuroScopePlugIn.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CTagSensePlugIn  * gpMyPlugin = NULL ;

//---EuroScopePlugInInit-----------------------------------------------

void    __declspec ( dllexport )    EuroScopePlugInInit ( EuroScopePlugIn :: CPlugIn ** ppPlugInInstance )
{
    AFX_MANAGE_STATE ( AfxGetStaticModuleState ())

    // create the instance
    * ppPlugInInstance = gpMyPlugin = new CTagSensePlugIn () ;
}


//---EuroScopePlugInExit-----------------------------------------------

void    __declspec ( dllexport )    EuroScopePlugInExit ( void )
{
    AFX_MANAGE_STATE ( AfxGetStaticModuleState ())

    // delete the instance
    delete gpMyPlugin ;
}

