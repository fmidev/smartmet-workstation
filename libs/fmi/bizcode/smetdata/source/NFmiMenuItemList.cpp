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

#include <boost/algorithm/string.hpp>
#include <cassert>

NFmiMenuItemList::NFmiMenuItemList(void)
:itsMenuItemList()
{
}

NFmiMenuItemList::NFmiMenuItemList(NFmiParamBag* theParamBag)
:itsMenuItemList()
{
	if(theParamBag)
	{
		theParamBag -> Reset();
		
		while(theParamBag -> NextActive())
		{
			auto menuItem = std::make_unique<NFmiMenuItem>(std::string(theParamBag->Current()->GetParamName()), FmiParameterName(theParamBag -> Current() -> GetParam()->GetIdent()));
			if (theParamBag->Current()->HasDataParams())
			{
				NFmiMenuItemList* menuItemList = new NFmiMenuItemList(theParamBag -> Current() -> GetDataParams());
				menuItem->AddSubMenu(menuItemList);
			}
            Add(std::move(menuItem));
        }
        SortParamsInAlphabeticalOrder();
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
        auto menuItem = std::make_unique<NFmiMenuItem>(theMapViewDescTopIndex, std::string(thePossibleMetaParam->GetParamName()),
            *thePossibleMetaParam, theMenuCommandType, theViewType, nullptr, theDataType);
        menuItemList.Add(std::move(menuItem));
    }
}

static void AddPossibleMetaParams(NFmiMenuItemList &menuItemList,
    int theMapViewDescTopIndex,
    const FmiMenuCommandType &theMenuCommandType,
    const NFmiMetEditorTypes::View &theViewType,
    NFmiInfoData::Type theDataType,
    const std::vector<std::unique_ptr<NFmiDataIdent>> *possibleMetaParams,
    bool paramAddedAlready)
{
    if(possibleMetaParams)
    {
        for(const auto &metaParamPtr : *possibleMetaParams)
        {
            if(metaParamPtr)
                ::AddPossibleMetaParam(menuItemList, theMapViewDescTopIndex, theMenuCommandType, theViewType, theDataType, metaParamPtr.get(), paramAddedAlready);
        }
    }
}

NFmiMenuItemList::NFmiMenuItemList(int theMapViewDescTopIndex, NFmiParamBag* theParamBag,
    const FmiMenuCommandType &theMenuCommandType,
    const NFmiMetEditorTypes::View &theViewType,
    NFmiInfoData::Type theDataType,
    const std::vector<std::unique_ptr<NFmiDataIdent>> *possibleMetaParams)
    :itsMenuItemList()
{
    if(theParamBag)
    {
        theParamBag->Reset();
        while(theParamBag->Next())
        {
            auto menuItem = std::make_unique<NFmiMenuItem>(theMapViewDescTopIndex, std::string(theParamBag->Current()->GetParamName()),
                *(theParamBag->Current()), theMenuCommandType, theViewType, nullptr, theDataType);

            if(theParamBag->Current()->HasDataParams())
            {
                NFmiMenuItemList* menuItemList =
                    new NFmiMenuItemList(theMapViewDescTopIndex, theParamBag->Current()->GetDataParams(),
                        theMenuCommandType, theViewType, theDataType);
                menuItem->AddSubMenu(menuItemList);
            }
            Add(std::move(menuItem));
        }

        ::AddPossibleMetaParams(*this, theMapViewDescTopIndex, theMenuCommandType, theViewType, theDataType, possibleMetaParams, false);
        SortParamsInAlphabeticalOrder();
    }
}

NFmiMenuItemList::NFmiMenuItemList(int theMapViewDescTopIndex, const NFmiDataIdent& theDataIdent, 
					const FmiMenuCommandType& theMenuCommandType, NFmiLevelBag* theLevels
					,NFmiInfoData::Type theDataType)
:itsMenuItemList()
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
		for(theLevels->Reset(); theLevels->Next();)
		{
			const NFmiLevel* level = theLevels->Level();
			std::string menuItemText("L ");
			menuItemText += NFmiStringTools::Convert(level->LevelValue());

            auto menuItem = std::make_unique<NFmiMenuItem>(theMapViewDescTopIndex, menuItemText, theDataIdent
				, theMenuCommandType, NFmiMetEditorTypes::View::kFmiParamsDefaultView, level, theDataType);
			Add(std::move(menuItem));
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
    , const NFmiDataIdent *thePossibleMetaParam)
{
    if(thePossibleMetaParam)
    {
        auto menuItem = std::make_unique<NFmiMenuItem>(theMapViewDescTopIndex, std::string(thePossibleMetaParam->GetParamName()),
            *thePossibleMetaParam, theMenuCommandType, theViewType, nullptr, theDataType);
        NFmiMenuItemList* subMenuItemList = new NFmiMenuItemList(theMapViewDescTopIndex, *thePossibleMetaParam
            , theMenuCommandType, theLevels, theDataType);
        menuItem->AddSubMenu(subMenuItemList);
        menuItemList.Add(std::move(menuItem));
    }
}

static void AddPossibleMetaParams(NFmiMenuItemList &menuItemList
    , int theMapViewDescTopIndex
    , const FmiMenuCommandType &theMenuCommandType
    , const NFmiMetEditorTypes::View &theViewType
    , NFmiLevelBag* theLevels
    , NFmiInfoData::Type theDataType
    , const std::vector<std::unique_ptr<NFmiDataIdent>> *possibleMetaParams)
{
    if(possibleMetaParams)
    {
        for(const auto &metaParamPtr : *possibleMetaParams)
        {
            if(metaParamPtr)
                ::AddPossibleMetaParam(menuItemList, theMapViewDescTopIndex, theMenuCommandType, theViewType, theLevels, theDataType, metaParamPtr.get());
        }
    }
}

NFmiMenuItemList::NFmiMenuItemList(int theMapViewDescTopIndex, NFmiParamBag* theParamBag
								  ,const FmiMenuCommandType &theMenuCommandType
								  ,const NFmiMetEditorTypes::View &theViewType
								  ,NFmiLevelBag* theLevels
								  ,NFmiInfoData::Type theDataType
								  ,FmiParameterName notLevelParam
                                  ,const std::vector<std::unique_ptr<NFmiDataIdent>> *possibleMetaParams)
:itsMenuItemList()
{
	if(theParamBag && theLevels)
	{
        for(theParamBag->Reset(); theParamBag->Next();)
		{
            auto dataIdent = theParamBag->Current();
			FmiParameterName param = FmiParameterName(dataIdent->GetParam()->GetIdent());
			if(param == notLevelParam)
			{
                auto menuItem2 = std::make_unique<NFmiMenuItem>(theMapViewDescTopIndex, std::string(dataIdent->GetParamName()),
									*dataIdent, theMenuCommandType, theViewType, nullptr, theDataType);
				Add(std::move(menuItem2));
			}
			else
			{
                MenuItem menuItem = std::make_unique<NFmiMenuItem>(theMapViewDescTopIndex, std::string(dataIdent->GetParamName()),
								*(dataIdent), theMenuCommandType, theViewType, nullptr, theDataType);

				if (dataIdent->HasDataParams())
				{
					NFmiMenuItemList* menuItemList = new NFmiMenuItemList(theMapViewDescTopIndex, dataIdent->GetDataParams(),
												    theMenuCommandType,theViewType, theLevels, theDataType);
                    menuItem -> AddSubMenu(menuItemList);
				}
				else
				{
					NFmiMenuItemList* menuItemList = new NFmiMenuItemList(theMapViewDescTopIndex, *dataIdent
														, theMenuCommandType, theLevels, theDataType);
					menuItem -> AddSubMenu(menuItemList);
				}
                Add(std::move(menuItem));
            }
		}

        ::AddPossibleMetaParams(*this, theMapViewDescTopIndex, theMenuCommandType, theViewType, theLevels, theDataType, possibleMetaParams);
        SortParamsInAlphabeticalOrder();
    }
}

NFmiMenuItemList::~NFmiMenuItemList(void)
{
}

bool NFmiMenuItemList::InitializeCommandIDs(unsigned long theFirstCommandID)
{
	unsigned long iD = theFirstCommandID;

	for(auto &menuItemPtr : itsMenuItemList)
	{
		if (menuItemPtr->SubMenu())
		{
            menuItemPtr->SubMenu()->InitializeCommandIDs(iD);
            menuItemPtr->SubMenu()->CalcMinAndMaxId();
			if(menuItemPtr->SubMenu()->MaxId())
				iD = (menuItemPtr->SubMenu()->MaxId()) + 1;
		}
		else
		{
            menuItemPtr->CommandId(iD);
			iD++;
		}
	}

	CalcMinAndMaxId();

	return true;
}

// Kun tämän luokan oliosta rakennetaan lopullista MFC:n CMenu oliota, pitää varmistaa ettei
// puurakenne ole liian iso. Olen huomannut, että jos rakenteessa on yli 100000 itemia,
// lakkaa popup menu toimimasta totaalisesti ja se ei tule edes näkyviin.
// FixOverSizedMenuTree metodi tekee siis seuraavaa:
// 1. Laske kuinka monta itemia on menu-puussa.
// 2. Jos lukema on liian iso, karsi puun lopusta ali puita kunnes päästään alle kriittisen rajan.
// 3. Lisää karsimistapauksessa päätasolle loppuun menu-item, jonka teksti varoittaa että puuta on karsittu.
// Palauttaa stringin, jossa on listattu poistetut rakenteet (lokia varten).
std::string NFmiMenuItemList::FixOverSizedMenuTree()
{
	const size_t criticalMaxMenuItemCount = 100000;
	auto totalMenuItemCount = TotalNumberOfMenuItems();
	if(totalMenuItemCount > criticalMaxMenuItemCount)
	{
		auto removedSubTreenames = PruneMenuTreeFromEnd(totalMenuItemCount - criticalMaxMenuItemCount);
		AddPruneWarningAsMainLevelMenuItem(removedSubTreenames);
		return removedSubTreenames;
	}
	return "";
}

// Deletoi menu-puun lopusta haluttu määrä (pruneCount) menu-itemeja.
// Tarkemmin sanottuna siivoa sitä päätason menu-puun oksaa, jossa
// on eniten itemeja (Esim. parametrin lisäyksessä Add -alimenu)
// ja sen alipuun lopusta alkaen.
// Jos poistetaan joku/joitain oks(i)a puusta, palauttaa kuvausen että:
// REMOVED from 'menu-item-name': sub-tree1, sub-tree2, ...
std::string NFmiMenuItemList::PruneMenuTreeFromEnd(size_t pruneCount)
{
	std::string removedSubTreeNames;
	auto biggestSubMenuInfo = GetBiggestSubMenuTree();
	if(biggestSubMenuInfo.second != nullptr)
	{
		removedSubTreeNames += "REMOVED from '" + biggestSubMenuInfo.first + "': ";
		size_t removedMenuItemCount = 0;
		auto& subMenuList = biggestSubMenuInfo.second->itsMenuItemList;
		for(auto reverseIter = std::rbegin(subMenuList); reverseIter != std::rend(subMenuList); )
		{
			if(!removedSubTreeNames.empty())
				removedSubTreeNames += ", ";
			removedSubTreeNames += (*reverseIter)->MenuText();
			removedMenuItemCount += (*reverseIter)->TotalNumberOfMenuItems();
			// Listasta poistaminen reverse iteraattoreita käyttäen on astetta vaikeampaa hommaa, kun ollaan vielä for-loopissa.
			// https://stackoverflow.com/questions/37005449/how-to-call-erase-with-a-reverse-iterator-using-a-for-loop
			reverseIter = decltype(reverseIter){ subMenuList.erase(std::next(reverseIter).base()) };

			if(removedMenuItemCount >= pruneCount)
				break;
		}
	}
	return removedSubTreeNames;
}

// Palauttaa parin, missä on isoimman alimenun nimi ja sen osoittama lista.
std::pair<std::string, NFmiMenuItemList*> NFmiMenuItemList::GetBiggestSubMenuTree()
{
	std::pair<std::string, NFmiMenuItemList*> biggestSubMenuTreeInfo{"", nullptr};
	size_t biggestSubMenuTreeSize = 0;
	for(auto& menuItem : itsMenuItemList)
	{
		auto* subMenu = menuItem->SubMenu();
		if(subMenu)
		{
			auto subMenuSize = subMenu->TotalNumberOfMenuItems();
			if(subMenuSize > biggestSubMenuTreeSize)
			{
				biggestSubMenuTreeSize = subMenuSize;
				biggestSubMenuTreeInfo.first = menuItem->MenuText();
				biggestSubMenuTreeInfo.second = subMenu;
			}
		}
	}
	return biggestSubMenuTreeInfo;
}

void NFmiMenuItemList::AddPruneWarningAsMainLevelMenuItem(const std::string& removedSubTreenames)
{
	Add(std::make_unique<NFmiMenuItem>(0, removedSubTreenames, kFmiBadParameter, kFmiNoCommand, NFmiMetEditorTypes::View::kFmiTextView, nullptr, NFmiInfoData::kNoDataType));
}

bool NFmiMenuItemList::Add(MenuItem &&theMenuItem)
{
	itsMenuItemList.push_back(std::move(theMenuItem));
    return true;
}

void NFmiMenuItemList::Clear()
{
	itsMenuItemList.clear();
}

bool NFmiMenuItemList::Find(const long &theId)
{
    itsRecursivelyFoundMenuItem = nullptr;
	for(auto &menuItem : itsMenuItemList)
	{
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
				itsRecursivelyFoundMenuItem = menuItem.get();
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

int NFmiMenuItemList::NumberOfSubMenus(int theNumberOfSubmenus)
{
    for(auto &menuItem : itsMenuItemList)
    {
		if(menuItem->SubMenu())
		{
			theNumberOfSubmenus = menuItem->SubMenu()->NumberOfSubMenus(theNumberOfSubmenus);
			theNumberOfSubmenus++;
		}
	}
	return theNumberOfSubmenus;
}

size_t NFmiMenuItemList::NumberOfMenuItems(void)
{
	return itsMenuItemList.size();
}

size_t NFmiMenuItemList::TotalNumberOfMenuItems() const
{
	size_t totalMenuItemCount = 0;
	for(auto& menuItem : itsMenuItemList)
	{
		totalMenuItemCount += menuItem->TotalNumberOfMenuItems();
	}
	return totalMenuItemCount;
}

void NFmiMenuItemList::CalcMinAndMaxId(void)
{
    for(auto &menuItem : itsMenuItemList)
    {
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

NFmiMenuItemList::MenuItemList::iterator NFmiMenuItemList::begin()
{
	return itsMenuItemList.begin();
}

NFmiMenuItemList::MenuItemList::iterator NFmiMenuItemList::end()
{
    return itsMenuItemList.end();
}

unsigned int NFmiMenuItemList::MinId(void)
{
	return itsMinId;
}

unsigned int NFmiMenuItemList::MaxId(void)
{
	return itsMaxId;
}

void NFmiMenuItemList::SortParamsInAlphabeticalOrder()
{
    itsMenuItemList.sort(
        [](const auto &menuItemPtr1, const auto &menuItemPtr2) 
    {
        return boost::algorithm::ilexicographical_compare(menuItemPtr1->MenuText(), menuItemPtr2->MenuText());
    }
    );
}
