//**********************************************************
// C++ Class Name : NFmiMenuItem 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiMenuItem.cpp 
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
// Changed 1999.08.25/Marko	Lisäsin uuden konstruktorin, ja data osan -levelin.
// Changed 1999.09.22/Marko	Lisäsin dataosaksi smartinfon datatyypin.
// 
//**********************************************************
#include <iostream>

#include "NFmiMenuItem.h"
#include "NFmiMenuItemList.h"
#include "NFmiLevel.h"

//--------------------------------------------------------
// AddSubMenu 
//--------------------------------------------------------

NFmiMenuItem::NFmiMenuItem(int theMapViewDescTopIndex
						  ,const std::string &theText
						  ,const NFmiDataIdent& theDataIdent
						  ,const FmiMenuCommandType &theMenuCommandType
						  ,const NFmiMetEditorTypes::View &theViewType
						  ,const NFmiLevel* theLevel
						  ,NFmiInfoData::Type theDataType
						  ,int theIndexInViewRow
						  ,bool viewMacroDrawParam)
:itsDataIdent(theDataIdent)
,itsCommandType(theMenuCommandType)
,itsMenuText(theText)
,itsParameter(FmiParameterName(theDataIdent.GetParam()->GetIdent()))	// Lieneekö oikein???
,itsViewType(theViewType)
,itsLevel(theLevel ? new NFmiLevel(*theLevel) : nullptr )
,itsSubMenu()
,itsDataType(theDataType)
,itsMapViewDescTopIndex(theMapViewDescTopIndex)
,itsIndexInViewRow(theIndexInViewRow)
,fViewMacroDrawParam(viewMacroDrawParam)
,itsExtraParam(0)
{
}

NFmiMenuItem::NFmiMenuItem(int theMapViewDescTopIndex, const std::string &theText
						  ,const FmiParameterName& theParam
						  ,const FmiMenuCommandType &theMenuCommandType
						  ,const NFmiMetEditorTypes::View &theViewType
						  ,const NFmiLevel* theLevel
						  ,NFmiInfoData::Type theDataType
						  ,int theIndexInViewRow
						  ,bool viewMacroDrawParam)
:itsDataIdent()
,itsCommandType(theMenuCommandType)
,itsMenuText(theText)
,itsParameter(theParam)
,itsViewType(theViewType)
,itsLevel(theLevel ? new NFmiLevel(*theLevel) : nullptr)
,itsSubMenu(nullptr)
,itsDataType(theDataType)
,itsMapViewDescTopIndex(theMapViewDescTopIndex)
,itsIndexInViewRow(theIndexInViewRow)
,fViewMacroDrawParam(viewMacroDrawParam)
,itsExtraParam(0)
{
}

NFmiMenuItem::~NFmiMenuItem()
{
}

const FmiMenuCommandType &NFmiMenuItem::CommandType(void) const
{
	return itsCommandType;
}

void NFmiMenuItem::CommandType(FmiMenuCommandType newCommandType)
{
	itsCommandType = newCommandType;
}

const std::string &NFmiMenuItem::MenuText(void) const
{
	return itsMenuText;
}

const FmiParameterName &NFmiMenuItem::Parameter(void) const
{
	return itsParameter;
}

const NFmiMetEditorTypes::View &NFmiMenuItem::ViewType(void) const
{
	return itsViewType;
}

const long& NFmiMenuItem::CommandId(void) const
{
	return itsCommandId;
}

NFmiMenuItemList* NFmiMenuItem::SubMenu(void) const
{
	return itsSubMenu.get();
}

void NFmiMenuItem::CommandId(long theCommandId)
{
	itsCommandId = theCommandId;
}


bool NFmiMenuItem::AddSubMenu (NFmiMenuItemList* theSubMenu)
{
	itsSubMenu.reset(theSubMenu);
	itsCommandId = 0;
	return true;
}

int NFmiMenuItem::IndexInViewRow(void) const
{
	return itsIndexInViewRow;
}

void NFmiMenuItem::IndexInViewRow(int newValue)
{
	itsIndexInViewRow = newValue;
}

size_t NFmiMenuItem::NumberOfSubMenuItems(void)
{
	if(itsSubMenu)
		return itsSubMenu->NumberOfMenuItems();

	return 0;
}

const NFmiLevel* NFmiMenuItem::Level(void) const
{ 
    return itsLevel.get(); 
}

size_t NFmiMenuItem::TotalNumberOfMenuItems() const
{
	// this olio on jo 1 item
	size_t totalNumberOfMenuItems = 1; 
	if(itsSubMenu)
	{
		totalNumberOfMenuItems += itsSubMenu->TotalNumberOfMenuItems();
	}
	return totalNumberOfMenuItems;
}
