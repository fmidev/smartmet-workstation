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

NFmiMenuItem::NFmiMenuItem(void)
:itsDataIdent()
,itsCommandType(kFmiShowView)
,itsText()
,itsParameter(kFmiLastParameter)
,itsViewType(NFmiMetEditorTypes::kFmiSymbolView)
,itsLevel(0)
,itsCommandId(0)
,itsSubMenu(0)
,itsDataType(NFmiInfoData::kEditable)
,itsMapViewDescTopIndex(-1)
,itsIndexInViewRow(-1)
,fViewMacroDrawParam(false)
,itsExtraParam(0)
{
}

NFmiMenuItem::NFmiMenuItem(const NFmiString &theText, const FmiParameterName& theParam)
:itsDataIdent()
,itsCommandType(kFmiShowView)
,itsText(theText)
,itsParameter(theParam)
,itsViewType(NFmiMetEditorTypes::kFmiSymbolView)
,itsLevel(0)
,itsSubMenu(0)
,itsDataType(NFmiInfoData::kEditable)
,itsMapViewDescTopIndex(-1)
,itsIndexInViewRow(-1)
,fViewMacroDrawParam(false)
,itsExtraParam(0)
{

}
NFmiMenuItem::NFmiMenuItem(int theMapViewDescTopIndex
						  ,const NFmiString &theText
						  ,const NFmiDataIdent& theDataIdent
						  ,const FmiMenuCommandType &theMenuCommandType
						  ,const NFmiMetEditorTypes::View &theViewType
						  ,const NFmiLevel* theLevel
						  ,NFmiInfoData::Type theDataType
						  ,int theIndexInViewRow
						  ,bool viewMacroDrawParam)
:itsDataIdent(theDataIdent)
,itsCommandType(theMenuCommandType)
,itsText(theText)
,itsParameter(FmiParameterName(theDataIdent.GetParam()->GetIdent()))	// Lieneekö oikein???
,itsViewType(theViewType)
,itsLevel(theLevel ? new NFmiLevel(*theLevel) : 0 )
,itsSubMenu(0)
,itsDataType(theDataType)
,itsMapViewDescTopIndex(theMapViewDescTopIndex)
,itsIndexInViewRow(theIndexInViewRow)
,fViewMacroDrawParam(viewMacroDrawParam)
,itsExtraParam(0)
{
}

NFmiMenuItem::NFmiMenuItem(int theMapViewDescTopIndex, const NFmiString &theText
						  ,const FmiParameterName& theParam
						  ,const FmiMenuCommandType &theMenuCommandType
						  ,const NFmiMetEditorTypes::View &theViewType
						  ,const NFmiLevel* theLevel
						  ,NFmiInfoData::Type theDataType
						  ,int theIndexInViewRow
						  ,bool viewMacroDrawParam)
:itsDataIdent()
,itsCommandType(theMenuCommandType)
,itsText(theText)
,itsParameter(theParam)
,itsViewType(theViewType)
,itsLevel(theLevel ? new NFmiLevel(*theLevel) : 0)
,itsSubMenu(0)
,itsDataType(theDataType)
,itsMapViewDescTopIndex(theMapViewDescTopIndex)
,itsIndexInViewRow(theIndexInViewRow)
,fViewMacroDrawParam(viewMacroDrawParam)
,itsExtraParam(0)
{
}

NFmiMenuItem::~NFmiMenuItem()
{
	delete itsLevel;
	delete itsSubMenu;
}

const FmiMenuCommandType &NFmiMenuItem::CommandType(void) const
{
	return itsCommandType;
}

const NFmiString &NFmiMenuItem::Text(void) const
{
	return itsText;
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
	return itsSubMenu;
}

void NFmiMenuItem::CommandId(long theCommandId)
{
	itsCommandId = theCommandId;
}


bool NFmiMenuItem::AddSubMenu (NFmiMenuItemList* theSubMenu)
{
	itsSubMenu = theSubMenu;
	itsCommandId = 0;
	return true;
}


void NFmiMenuItem::Print(int roundCheck) const			// Tämä on vain testausta varten.
{
	using namespace std;

	if(itsSubMenu)
	{
		for (int i = 1; i < roundCheck; i++)
			cout << "          ";
		cout << itsText.CharPtr();
		cout << endl;
		roundCheck++;
		itsSubMenu->Print(roundCheck);
	}
	else
	{
		for (int i = 1; i < roundCheck; i++)
			cout << "          ";
		cout << itsText.CharPtr();
		cout << "   ";
		cout << itsCommandId << endl;
	}
	return;
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