//**********************************************************
// C++ Class Name : NFmiMenuItemList 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiMenuItemList.cpp 
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
#include "NFmiMenuItemList.h"

#include "NFmiMenuItem.h"
#include "NFmiDrawParamList.h"
#include "NFmiDrawParam.h"
#include "NFmiParamBag.h"
#include "NFmiLevelBag.h"
#include <cassert>

NFmiMenuItemList::NFmiMenuItemList(void)
:itsList()
,itsIter(itsList.Start())
{
}

NFmiMenuItemList::NFmiMenuItemList(NFmiParamBag* theParamBag)
:itsList()
,itsIter(itsList.Start())
{
	if(theParamBag)
	{
		theParamBag -> Reset();
		
		while(theParamBag -> NextActive())
		{
			NFmiMenuItem* menuItem;
			menuItem = new NFmiMenuItem(theParamBag->Current()->GetParamName(), 
								FmiParameterName(theParamBag -> Current() -> GetParam()->GetIdent()));
			Add(menuItem);

			if (theParamBag->Current()->HasDataParams())
			{
				NFmiMenuItemList* menuItemList;
				menuItemList = 
					new NFmiMenuItemList(theParamBag -> Current() -> GetDataParams());
				menuItem -> AddSubMenu(menuItemList);
			}
		}
	}
}

static void AddPossibleMetaParam(NFmiMenuItemList &menuItemList,
    int theMapViewDescTopIndex,
    const FmiMenuCommandType &theMenuCommandType,
    const NFmiMetEditorTypes::View &theViewType,
    NFmiInfoData::Type theDataType,
    const NFmiDataIdent *thePossibleMetaParam,
    bool paramAddedAlready)
{
    if(thePossibleMetaParam && !paramAddedAlready)
    {
        NFmiMenuItem* menuItem = new NFmiMenuItem(theMapViewDescTopIndex, thePossibleMetaParam->GetParamName(),
            *thePossibleMetaParam, theMenuCommandType, theViewType, 0, theDataType);
        menuItemList.Add(menuItem);
    }
}

NFmiMenuItemList::NFmiMenuItemList(int theMapViewDescTopIndex, NFmiParamBag* theParamBag, 
									const FmiMenuCommandType &theMenuCommandType,
									const NFmiMetEditorTypes::View &theViewType,
									NFmiInfoData::Type theDataType, 
                                    const NFmiDataIdent *thePossibleStreamLineParam,
                                    const NFmiDataIdent *thePossibleWindVectorParam)
:itsList()
,itsIter(itsList.Start())
{
	if(theParamBag)
	{
        bool streamLineParamAdded = false;
		theParamBag -> Reset();
		while(theParamBag -> Next())
		{
			NFmiMenuItem* menuItem = new NFmiMenuItem(theMapViewDescTopIndex, theParamBag -> Current() -> GetParamName(), 
							*(theParamBag->Current()), theMenuCommandType, theViewType, 0, theDataType);
			Add(menuItem);			

			if (theParamBag -> Current() -> HasDataParams())
			{
				NFmiMenuItemList* menuItemList;
				menuItemList = 
					new NFmiMenuItemList(theMapViewDescTopIndex, theParamBag -> Current() -> GetDataParams(),
                    theMenuCommandType,theViewType, theDataType, thePossibleStreamLineParam);
                if(thePossibleStreamLineParam && theParamBag->Current()->GetParamIdent() == kFmiTotalWindMS)
                {
                    streamLineParamAdded = true;
                }
				menuItem -> AddSubMenu(menuItemList);
			}
		}
        ::AddPossibleMetaParam(*this, theMapViewDescTopIndex, theMenuCommandType, theViewType, theDataType, thePossibleWindVectorParam, false);
        ::AddPossibleMetaParam(*this, theMapViewDescTopIndex, theMenuCommandType, theViewType, theDataType, thePossibleStreamLineParam, streamLineParamAdded);
	}
}

NFmiMenuItemList::NFmiMenuItemList(int theMapViewDescTopIndex, const NFmiDataIdent& theDataIdent, 
					const FmiMenuCommandType& theMenuCommandType, NFmiLevelBag* theLevels
					,NFmiInfoData::Type theDataType)
:itsList()
,itsIter(itsList.Start())
{
	if(theLevels)
	{
        // Windows has limits in the menu resources, so you can't create overly excessive sized popup menus.
        // Some data migth have hundreds or even thousands of levels which will generate huge amount of menu items.
        // Let's limit generated level count here in order to avoid resource problems. This is done because 
        // high resolution ship sounding data caused the popup to act strangely. There were about 7700 levels
        // with 10 parameters which combined with other scand data resulted over 170000 menuitems. Problem limit
        // on Windows 10 (64-bit) is between 85000 and 170000 menu items.
        const int maxLevelsIncludedInPopup = 250;
        int levelCounter = 0;
		NFmiMenuItem* menuItem;
		for(theLevels->Reset(); theLevels->Next();)
		{
			const NFmiLevel* level = theLevels->Level();
			NFmiString menuItemText("L ");
			menuItemText += NFmiStringTools::Convert(level->LevelValue());

			menuItem = new NFmiMenuItem(theMapViewDescTopIndex, menuItemText, theDataIdent
				, theMenuCommandType, NFmiMetEditorTypes::kFmiParamsDefaultView, level, theDataType);
			Add(menuItem);
            levelCounter++;
            if(levelCounter >= maxLevelsIncludedInPopup)
                break;
		}
	}
}

static void AddPossibleMetaParam(NFmiMenuItemList &menuItemList
    , int theMapViewDescTopIndex
    , const FmiMenuCommandType &theMenuCommandType
    , const NFmiMetEditorTypes::View &theViewType
    , NFmiLevelBag* theLevels
    , NFmiInfoData::Type theDataType
    , const NFmiDataIdent *thePossibleMetaParam
    , bool paramAddedAlready)
{
    if(thePossibleMetaParam && !paramAddedAlready)
    {
        NFmiMenuItem *menuItem = new NFmiMenuItem(theMapViewDescTopIndex, thePossibleMetaParam->GetParamName(),
            *thePossibleMetaParam, theMenuCommandType, theViewType, 0, theDataType);
        menuItemList.Add(menuItem);
        NFmiMenuItemList* menuItemList = new NFmiMenuItemList(theMapViewDescTopIndex, *thePossibleMetaParam
            , theMenuCommandType, theLevels, theDataType);
        menuItem->AddSubMenu(menuItemList);
    }
}

NFmiMenuItemList::NFmiMenuItemList(int theMapViewDescTopIndex, NFmiParamBag* theParamBag
								  ,const FmiMenuCommandType &theMenuCommandType
								  ,const NFmiMetEditorTypes::View &theViewType
								  ,NFmiLevelBag* theLevels
								  ,NFmiInfoData::Type theDataType
								  ,FmiParameterName notLevelParam
                                  ,const NFmiDataIdent *thePossibleStreamLineParam
                                  ,const NFmiDataIdent *thePossibleWindVectorParam)
:itsList()
,itsIter(itsList.Start())
{
	if(theParamBag && theLevels)
	{
        bool streamLineParamAdded = false;
        for(theParamBag->Reset(); theParamBag->Next();)
		{
            auto dataIdent = theParamBag->Current();
			NFmiMenuItem* menuItem = nullptr;
			FmiParameterName param = FmiParameterName(dataIdent->GetParam()->GetIdent());
			if(param == notLevelParam)
			{
				NFmiMenuItem *menuItem2 = new NFmiMenuItem(theMapViewDescTopIndex, dataIdent->GetParamName(),
									*dataIdent, theMenuCommandType, theViewType, 0, theDataType);
				Add(menuItem2);
			}
			else
			{
				menuItem = new NFmiMenuItem(theMapViewDescTopIndex, dataIdent->GetParamName(),
								*(dataIdent), theMenuCommandType, theViewType, 0, theDataType);
				Add(menuItem);			

				if (dataIdent->HasDataParams())
				{
					NFmiMenuItemList* menuItemList = 0;
                    if(thePossibleStreamLineParam && dataIdent->GetParamIdent() == kFmiTotalWindMS)
                    {
                        streamLineParamAdded = true;
                        NFmiParamBag tmpParamBag(*dataIdent->GetDataParams());
                        tmpParamBag.Add(*thePossibleStreamLineParam);
					    menuItemList = 
						    new NFmiMenuItemList(theMapViewDescTopIndex, &tmpParamBag,
												    theMenuCommandType, theViewType, theLevels, theDataType);
                    }
                    else
                    {
					    menuItemList = 
						    new NFmiMenuItemList(theMapViewDescTopIndex, dataIdent->GetDataParams(),
												    theMenuCommandType,theViewType, theLevels, theDataType);
                    }
                    menuItem -> AddSubMenu(menuItemList);
				}
				else
				{
					NFmiMenuItemList* menuItemList = new NFmiMenuItemList(theMapViewDescTopIndex, *dataIdent
														, theMenuCommandType, theLevels, theDataType);
					menuItem -> AddSubMenu(menuItemList);
				}
			}
		}
        ::AddPossibleMetaParam(*this, theMapViewDescTopIndex, theMenuCommandType, theViewType, theLevels, theDataType, thePossibleWindVectorParam, false);
        ::AddPossibleMetaParam(*this, theMapViewDescTopIndex, theMenuCommandType, theViewType, theLevels, theDataType, thePossibleStreamLineParam, streamLineParamAdded);
	}
}

NFmiMenuItemList::NFmiMenuItemList(NFmiDrawParamList* theDrawParamList)
:itsList()
,itsIter(itsList.Start())
{
	if(theDrawParamList)
	{
		theDrawParamList->Reset();
		
		while(theDrawParamList->Next())
		{
			NFmiMenuItem* menuItem = 0;
			throw std::runtime_error("NFmiMenuItemList::NFmiMenuItemList ei toimi");
			Add(menuItem);
		}	
	}
}


NFmiMenuItemList::~NFmiMenuItemList(void)
{
	itsList.Clear(true);	// tuhoaa listan itemit
}

bool NFmiMenuItemList::InitializeCommandIDs(unsigned long theFirstCommandID)
{
	unsigned long iD = theFirstCommandID;

	NFmiMenuItem* menuItem;
	for(itsIter = itsList.Start(); itsIter.Next();)
	{
		menuItem = itsIter.CurrentPtr();
		if (menuItem->SubMenu())
		{
			menuItem->SubMenu()->InitializeCommandIDs(iD);
			menuItem->SubMenu()->CalcMinAndMaxId();
			if(menuItem->SubMenu()->MaxId())
				iD = (menuItem->SubMenu()->MaxId()) + 1;
		}
		else
		{
			menuItem->CommandId(iD);
			iD++;
		}
	}

	CalcMinAndMaxId();

	return true;
}

//--------------------------------------------------------
// Add 
//--------------------------------------------------------
bool NFmiMenuItemList::Add (NFmiMenuItem* theMenuItem)
{
	return itsList.AddEnd(theMenuItem);
}

void NFmiMenuItemList::Clear(bool fDeleteItem)
{
	itsList.Clear(fDeleteItem);
}

bool NFmiMenuItemList::Find(const long &theId)
{
    itsRecursivelyFoundMenuItem = nullptr;
	NFmiMenuItem* menuItem = nullptr;
	for(itsIter = itsList.Start(); itsIter.Next();)
	{
		menuItem = itsIter.CurrentPtr();
		if (menuItem->SubMenu())
		{
			if (menuItem->SubMenu()->Find(theId))
			{
				itsRecursivelyFoundMenuItem = menuItem->SubMenu()->RecursivelyFoundMenuItem();
				return true;
			}
		}
		else
		{
			if (menuItem->CommandId() == theId)
			{
				itsRecursivelyFoundMenuItem = menuItem;
				return true;
			}
		}
	}
	return false;
}

NFmiMenuItem* NFmiMenuItemList::RecursivelyFoundMenuItem(void)
{
	return itsRecursivelyFoundMenuItem;
}


void NFmiMenuItemList::Print(int roundCheck)			// Tämä on vain testausta varten.
{
	NFmiMenuItem* menuItem;
	for(itsIter = itsList.Start(); itsIter.Next();)
	{
		menuItem = itsIter.CurrentPtr();
		menuItem->Print(roundCheck);
	}

	return;
}

int NFmiMenuItemList::NumberOfSubMenus(int theNumberOfSubmenus)
{
	NFmiMenuItem* menuItem;
	for(itsIter = itsList.Start(); itsIter.Next();)
	{
		menuItem = itsIter.CurrentPtr();
		if (menuItem && menuItem->SubMenu())
		{
			theNumberOfSubmenus = menuItem->SubMenu()->NumberOfSubMenus(theNumberOfSubmenus);
			theNumberOfSubmenus++;
		}
	}
	return theNumberOfSubmenus;
}

unsigned long NFmiMenuItemList::NumberOfMenuItems(void)
{
	return itsList.NumberOfItems();
}

void NFmiMenuItemList::CalcMinAndMaxId(void)
{
	NFmiMenuItem* menuItem;
	for(itsIter = itsList.Start(); itsIter.Next();)
	{
		menuItem = itsIter.CurrentPtr();
		if (menuItem->CommandId() != 0)		// Jos menuItemilla on SubMenuja, on sen ID 0.
		{
			if (menuItem->CommandId() < static_cast<long>(itsMinId))
				itsMinId = menuItem->CommandId();
			if (menuItem->CommandId() > static_cast<long>(itsMaxId))
				itsMaxId = menuItem->CommandId();
		}
		if (menuItem->SubMenu())			// Voisi olla myös else-haara.
		{		
			menuItem->SubMenu()->CalcMinAndMaxId();
			if (menuItem->SubMenu()->MinId() < itsMinId)
				itsMinId = menuItem->SubMenu()->MinId();
			if (menuItem->SubMenu()->MaxId() > itsMaxId)
				itsMaxId = menuItem->SubMenu()->MaxId();
		}
	}
	return;	
}

void NFmiMenuItemList::Reset(void)
{
	itsIter = itsList.Start();
	return;
}

bool NFmiMenuItemList::Next(void)
{
	return itsIter.Next();
}

NFmiMenuItem *NFmiMenuItemList::CurrentMenuItem(void)
{
	return itsIter.CurrentPtr();
}

unsigned int NFmiMenuItemList::MinId(void)
{
	return itsMinId;
}

unsigned int NFmiMenuItemList::MaxId(void)
{
	return itsMaxId;
}
