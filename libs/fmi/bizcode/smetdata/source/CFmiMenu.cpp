
//**********************************************************
// C++ Class Name : CFmiMenu 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/CFmiMenu.cpp 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 2 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : Markon ehdotus 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Thur - Jan 28, 1999 
// 
// 
//  Description: 
// 
//  Change Log: 
// 
//**********************************************************
#include "CFmiMenu.h"
#include "NFmiMenuItemList.h"

#include "stdafx.h"

CFmiMenu::CFmiMenu()
:itsNumberOfSubMenus(0)
,itsMenus(0)
,itsMenuItemLists(0)
{
} 

CFmiMenu::~CFmiMenu()
{
	if (itsMenus)
	{
		delete [] itsMenus;
		itsMenus = NULL;
	}
	if (itsMenuItemLists)
	{
		delete [] itsMenuItemLists;
		itsMenuItemLists = 0;
	}
}


void CFmiMenu::Init(NFmiMenuItemList* theMenuItemList)
{
	if(!theMenuItemList)
		return;
	int listOrderNumber = 0;
	itsNumberOfSubMenus = theMenuItemList->NumberOfSubMenus();

	itsMenus = new CMenu[itsNumberOfSubMenus + 1];
	int menuNr;
	for (menuNr = 0; menuNr <= itsNumberOfSubMenus; menuNr++)
	{
		itsMenus[menuNr].CreatePopupMenu();
	}

	itsMenuItemLists = new NFmiMenuItemList*[itsNumberOfSubMenus + 1];

	itsMenuItemLists[0] = theMenuItemList;

	for (menuNr = 0; menuNr <= itsNumberOfSubMenus; menuNr++)
	{
		itsMenuItemLists[menuNr]->Reset();
		while (itsMenuItemLists[menuNr]->Next())
		{
			if (itsMenuItemLists[menuNr]->CurrentMenuItem()->SubMenu())
			{
				listOrderNumber++;
				itsMenuItemLists[listOrderNumber] 
					= itsMenuItemLists[menuNr]->CurrentMenuItem()->SubMenu();
				itsMenus[menuNr].InsertMenu(-1, MF_BYPOSITION | MF_STRING | MF_POPUP, 
					(UINT_PTR)itsMenus[listOrderNumber].GetSafeHmenu(), 
						CA2T(itsMenuItemLists[menuNr]->CurrentMenuItem()->Text()));
			}
			else
			{
				itsMenus[menuNr].InsertMenu(-1, MF_BYPOSITION | MF_STRING, 
					itsMenuItemLists[menuNr]->CurrentMenuItem()->CommandId(),
                    CA2T(itsMenuItemLists[menuNr]->CurrentMenuItem()->Text()));
			}
		}
			
	}
}

void CFmiMenu::Run(const CPoint &point)
{
	itsMenus[0].TrackPopupMenu(TPM_CENTERALIGN | TPM_RIGHTBUTTON, point.x, point.y, 
									AfxGetMainWnd());	
	return;
}

CMenu* CFmiMenu::Popup(void)
{
	return &(itsMenus[0]);
}

