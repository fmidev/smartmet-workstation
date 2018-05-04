//**********************************************************
// C++ Class Name : CFmiMenu 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/CFmiMenu.h 
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

#pragma once

class NFmiMenuItemList;
class CPoint;
class CMenu;

class CFmiMenu //: public CMenu
{

public:
	CFmiMenu(void);
	virtual ~CFmiMenu(void);
	void Init(NFmiMenuItemList* theMenuItemList);
	void Run(const CPoint &point);
	CMenu* Popup(void);

private:
	int itsNumberOfSubMenus;
	CMenu* itsMenus;
	NFmiMenuItemList** itsMenuItemLists;
};

